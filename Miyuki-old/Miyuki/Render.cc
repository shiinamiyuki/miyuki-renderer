#include "Render.h"
#include "MainWindow.h"
#include "kernel/trace.h"


void Render::allocateBuffer()
{
	Xi = new Seed[2 * width * height];
	for (int i = 0; i < width * height; i++) {
		*(int*)&Xi[i * 2] = rand();
	}
	cl_int ret;
	buffer.Xi.create(&context,
		CL_MEM_READ_WRITE,
		sizeof(Seed) * 2 * width*height);
	buffer.image.create(&context,
		CL_MEM_READ_WRITE,
		sizeof(Pixel) * width*height);
	buffer.nodes.create(&context,
		CL_MEM_READ_ONLY,
		sizeof(BVHNode)*bvhTree->nodes.size());
	buffer.primPool.create(&context,
		CL_MEM_READ_ONLY,
		sizeof(int)*bvhTree->prims.size());
	buffer.objects.create(&context,
		CL_MEM_READ_ONLY,
		sizeof(Primitive) * objects.size());
	buffer.lights.create(&context,
		CL_MEM_READ_ONLY,
		sizeof(int) * lights.size());
	buffer.renderCtx.create(&context,
		CL_MEM_READ_WRITE,
		sizeof(RenderContext));
	buffer.opt.create(&context,
		CL_MEM_READ_WRITE,
		sizeof(OptionalBuffer) * width*height);
	buffer.materials.create(&context,
		CL_MEM_READ_ONLY,
		sizeof(Material) * allMaterials.size());
	unsigned int _i = objects.size(),_j = lights.size();
	buffer.Xi.write(&context, sizeof(Seed) * 2 * width*height, Xi);
	buffer.objects.write(&context,sizeof(Primitive) * objects.size(), objects.data());
	buffer.lights.write(&context, sizeof(int) * lights.size(), lights.data());
	buffer.nodes.write(&context,sizeof(BVHNode)* bvhTree->nodes.size(), bvhTree->nodes.data());
	buffer.primPool.write(&context,	sizeof(int)* bvhTree->prims.size(), bvhTree->prims.data());
	buffer.materials.write(&context, sizeof(Material) * allMaterials.size(), allMaterials.data());
}

void Render::setArgs()
{

}

void Render::initContext()
{
	
}

void Render::finishSetup()
{
	frame = 0;
	globalSize = width * height;
	localSize = 256;
	sampleCount = 0;
	signal.hasNew = false;
	image = new Pixel[width * height];
	int id = 0;
	for (auto &object : objects) {
		object.id = id++;
	}
	for (auto &object : objects) {
		auto ambient = allMaterials[object.material].ambient;
		if (vmax(ambient) > 0.1) {
			lights.emplace_back(object.id);
		}
	}
	
	logger->log("Object count: {}\n", objects.size());
	if (lights.empty()) {
		logger->log("no light source\n");
	}
	logger->log("Materials: {}\n", allMaterials.size());
	logger->log("Lights: {}\n", lights.size());
	logger->log("Constructing BVH\n");
	bvhTree = new BVH(objects);
	std::set<int> set;
	for (auto i : bvhTree->prims) {
		set.insert(i);
	}
	logger->log("BVH Nodes:{}\n", bvhTree->nodes.size());
	logger->log("Expecting {}, found {}\n", objects.size(), set.size());
	logger->log("Allocating buffers\n");
	allocateBuffer();
	setArgs();
}
unsigned char toInt(float x) {
	x = (x < 0) ? 0 : (x > 1 ? 1 : x);
	return pow(x, 1 / 2.2) * 255 + 0.5;
}

void Render::copyImage(QPixmap & pixmap)
{
	buffer.image.read(&context,	width * height * sizeof(Pixel), image);
	clFinish(context.getCommandQueue());

	concurrency::parallel_for((unsigned int)0,
		(unsigned int)width * height, 
		[&](unsigned int i) {
		pixelBuffer[4 * i + 0] = toInt(image[i].x);
		pixelBuffer[4 * i + 1] = toInt(image[i].y);
		pixelBuffer[4 * i + 2] = toInt(image[i].z);
		pixelBuffer[4 * i + 3] = 255;
	});
	QImage image(pixelBuffer, width, height, QImage::Format::Format_RGBA8888);
	pixmap.convertFromImage(image);
}

void Render::resetSample()
{
	sampleCount = -10;
}

void Render::renderPass()
{
	//if (mode == MODE_PREVIEW && sampleCount >= 2)return;
	if (sampleCount < 0)
		sampleCount = 0;
	RenderContext ctx;
	ctx.samples = sampleCount;
	ctx.width = width;
	ctx.height = height;
	ctx.lightCount = lights.size();
	ctx.objectCount = objects.size();
	ctx.size = width * height;
	ctx.origin = pos;
	ctx.dir = dir;
	buffer.renderCtx.write(&context, sizeof(RenderContext), &ctx);
	integrator.curIntegrator().setWorkDimesion(globalSize, localSize);
	integrator.curIntegrator()(
		buffer.renderCtx,
		buffer.materials,
		buffer.objects,
		buffer.Xi,
		buffer.nodes,
		buffer.primPool,
		buffer.lights,
		buffer.image,
		buffer.opt);
	
	sampleCount++;
}


Render::Render(MainWindow *w):context(Logger(w))
{
	mode = MODE_PREVIEW;
	width = height = 1000;
	window = w;
	pixelBuffer = new unsigned char[width * height * 4];
	pos = vec3(250, 250, -350);
	dir = vec3(0, 0, 0);
	signal.alive = true;
	signal.hasNew = false;
	logger = new Logger(w);
	integrator.setContext(&context);
	integrator.load();
}


Render::~Render()
{
}
struct Polygon {
	std::vector<Vector> vertices;
	std::vector<Primitive> trigs;
	int material;

	Polygon() {}

	void addVertex(const Vector &vector) {
		vertices.emplace_back(vector);
	}

	const std::vector<Primitive> &getTrigs() {
		if (trigs.empty() && vertices.size() >= 3) {
			for (int i = 1; i < vertices.size() - 1; i++) {
				Primitive triangle;
				triangle.triangle.vertex0 = vertices[0];
				triangle.triangle.vertex1 = vertices[i];
				triangle.triangle.vertex2 = vertices[i + 1];
				QVector3D a, b;
				a = fromVec3(vertices[i]) - fromVec3(vertices[0]);
				b = fromVec3(vertices[i+1]) - fromVec3(vertices[0]);
				triangle.triangle.edge1 = toVec3(a);
				triangle.triangle.edge2 = toVec3(b);
				auto norm = QVector3D::crossProduct(a, b);
				triangle.area = norm.length() / 2;
				norm.normalize();
				triangle.triangle.norm = toVec3(norm);
				triangle.material = material;
				triangle.type = TYPE_TRIANGLE;
				trigs.emplace_back(triangle);
			}
		}
		return trigs;
	}
};

void Render::moveCameraBy(const QVector3D &v)
{
	pos = toVec3(fromVec3(pos) + v);
}

void Render::moveCameraTo(const QVector3D &v)
{
	pos = toVec3(v);
}

void Render::setCameraDir(const QVector3D &v)
{
	dir = toVec3(v);
}

void Render::loadObj(const char *filename) {
	fmt::print("Loading {0}\n", filename);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);

	if (!err.empty()) { // `err` may contain warning message.
		logger->log("{}\n", err);
	}

	if (!ret) {
		logger->log("cannot open file {}", filename);
		return;
	}
	logger->log("Adding objects...\n");
	std::vector<Polygon> polygons;
	for (int id = 0; id < materials.size(); id++) {
		Material material;
		material.Ni = 1;
		material.Tr = 0;
		material.roughness = materials[id].roughness;
		material.ambient.x = materials[id].emission[0];
		material.diffuse.x = materials[id].diffuse[0];
		material.specular.x = materials[id].specular[0];
		material.ambient.y = materials[id].emission[1];
		material.diffuse.y = materials[id].diffuse[1];
		material.specular.y = materials[id].specular[1];
		material.ambient.z = materials[id].emission[2];
		material.diffuse.z = materials[id].diffuse[2];
		material.specular.z = materials[id].specular[2];
		material.Ni = materials[id].ior;
		material.Tr = 1 - materials[id].dissolve;
		allMaterials.emplace_back(material);
	}
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			auto id = shapes[s].mesh.material_ids[f];
			/*Material material;
			material.Ni = 1;
			material.Tr = 0;
			if (id >= 0 && id < materials.size()) {
				material.ambient.x = materials[id].emission[0];
				material.diffuse.x = materials[id].diffuse[0];
				material.specular.x = materials[id].specular[0];
				material.ambient.y = materials[id].emission[1];
				material.diffuse.y = materials[id].diffuse[1];
				material.specular.y = materials[id].specular[1];
				material.ambient.z = materials[id].emission[2];
				material.diffuse.z = materials[id].diffuse[2];
				material.specular.z = materials[id].specular[2];
				material.Ni = materials[id].ior;
				material.Tr = 1 - materials[id].dissolve;
			}
			else {
				material.ambient.x = 0;
				material.diffuse.x = 1;
				material.specular.x = 0;
				material.ambient.y = 0;
				material.diffuse.y = 1;
				material.specular.y = 0;
				material.ambient.z = 0;
				material.diffuse.z = 1;
				material.specular.z = 0;
			}*/
			//fmt::print("{} {} {} {}\n",id, material.Ka,material.Kd,material.Ks);
			Polygon polygon;
			polygon.material = id;
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				/*   tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				   tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				   tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];*/
				  // tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				  // tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				  
				polygon.addVertex(vec3( vx, vy, vz ));
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
			}
			polygons.emplace_back(polygon);
			index_offset += fv;
		}
	}
	logger->log("Processing polygons...\n");
	for (auto &i : polygons) {
		auto trigs = i.getTrigs();
		for (auto &trig : trigs) {
			addObject(trig);
		}
	}
}

void Render::render()
{
	finishSetup();
	logger->log("Rendering\n");
	auto lastUpdate = std::chrono::system_clock::now();
	while (signal.alive) {
		auto start = std::chrono::system_clock::now();
		renderPass();
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> tillLast = end - lastUpdate;
		if(tillLast.count() >= 1.0/10.0)
			signal.hasNew = true;
		frame++;
		
		std::chrono::duration<double> elapsed_seconds = end - start;
		window->ui.status->setText(QString("Iteration: ").append(QString::number(sampleCount + 1)
			.append(" ").append(QString::number((width*height) / elapsed_seconds.count() / 1e6))
			.append("Msamples/sec")));
		do { std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}while (window->saving);
	}
}

Material makeEmission(const Vector &c)
{
	Material m;
	m.ambient = c;
	m.diffuse = m.specular = vec3(0, 0, 0);
	return m;
}

Material makeDiffuse(const Vector &c)
{
	Material m;
	m.diffuse = c;
	m.ambient = m.specular = vec3(0, 0, 0);
	m.Ni = 1;
	m.Tr = 0;
	return m;
}

Material makeSpec(const Vector &c,float r)
{
	Material m;
	m.specular = c;
	m.ambient = m.diffuse = vec3(0, 0, 0);
	m.Ni = 1;
	m.Tr = 0;
	m.roughness = r;
	return m;
}


Material makeRefr(const Vector &c, float ni)
{
	Material m;
	m.specular = c;
	m.ambient = m.diffuse = vec3(0, 0, 0);
	m.Ni = ni;
	m.Tr = 1;
	return m;
}

Primitive makeSphere(const Vector &c, float t, int m)
{
	Primitive p;
	p.type = TYPE_SPHERE;
	p.sphere.center = c;
	p.sphere.radius = t;
	p.material = m;
	return p;
}

void Render::Integrator::setContext(CoreCL::Context * ctx)
{
	pt.setContext(ctx);
	bdpt.setContext(ctx);
	ao.setContext(ctx);
	raycast.setContext(ctx);
}

void Render::Integrator::load()
{
	pt.createKernel("kernel/integrator/pt-next-event-estimation.cl","render");
	//bdpt.createKernel("kernel/integrator/bdpt.cl", "render");
	ao.createKernel("kernel/integrator/ambient-occlusion.cl", "render");
	raycast.createKernel("kernel/integrator/ray-cast.cl", "render");
}
