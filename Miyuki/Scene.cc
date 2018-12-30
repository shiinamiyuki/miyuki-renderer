#include "Scene.h"
#include "Primitive.h"
#include "aabb.h"

using namespace Miyuki;

void Miyuki::Scene::trace(int x0, int y0)
{
	
	screen[x0 + w * y0] *= sampleCount;
	screen[x0 + w * y0] += tracer.trace(x0, y0);
	screen[x0 + w * y0] /= (1 + sampleCount);
}

Ray Miyuki::Scene::cameraRay(int x, int y)
{
	return Ray(vec3(),vec3());
}



const char * getCurTime() {
	auto today = std::chrono::system_clock::now();
	auto t = std::chrono::system_clock::to_time_t(today);
	return ctime(&t);
}

void Miyuki::Scene::loadSessionData()
{
	std::ifstream in(sessionFilename);
	if (in.good()) {
		std::string src;
		while (!in.eof())src += in.get();
		sessionData = XMLReader(src).read();
	}
	else {
		sessionData = makeNode("SessionData");
		sessionData->addAttribute("version", "0.01");
		sessionData->addAttribute("create", getCurTime());
	}
	auto name = session.name;
	auto all = sessionData->findAllTags("session");
	for (auto i : all) {
		if (i->getAttribute("name") == name) {
			currentSession = i;
			break;
		}
	}
	
}

void Miyuki::Scene::restore()
{
	if (currentSession && !option.startNew) {
		session.w = currentSession->getFloatAttribute("w");
		session.h = currentSession->getFloatAttribute("h");
		session.cnt = currentSession->getFloatAttribute("sample");
		logger->log("Restoring session '{}' {}x{} \n", session.name, session.w, session.h);
		std::vector<unsigned char> pixel;
		std::string name = fmt::format("{}.png", session.name);
		unsigned int x, y;
		lodepng::decode(pixel, x, y, name);
		if (x != w || y != h) {
			logger->log("Failed to restore. Start new.\n");
			session.cnt = 0;
		}
		else {
			for (auto i = 0; i < w*h; i++) {
				screen[i] = vec3(pixel[4 * i], pixel[4 * i + 1], pixel[4 * i + 2]) / 255.0;
			}
		}
	}
	else {
		if (!currentSession) {
			currentSession = makeNode("session");
			sessionData->addNode(currentSession);
		}
	}
}

void Miyuki::Scene::saveSession()
{
	auto filename = sessionFilename;
	std::ofstream out(filename);
	session.cnt = sampleCount;
	currentSession->addAttribute("w", w);
	currentSession->addAttribute("h", h);
	currentSession->addAttribute("sample", session.cnt);
	currentSession->addAttribute("name", session.name);
	sessionData->addAttribute("lastsave", getCurTime());
	out << sessionData->toString();
	out.close();
	std::string png = fmt::format("{}.png", session.name);
	std::vector<unsigned char> pixel;
	for (auto i : screen) {
		pixel.push_back(static_cast<unsigned char>(toInt(i.x())));
		pixel.push_back(static_cast<unsigned char>(toInt(i.y())));
		pixel.push_back(static_cast<unsigned char>(toInt(i.z())));
		pixel.push_back(255);
	}
	lodepng::encode(png.c_str(), pixel, w, h);
}


void Miyuki::Scene::readScript(const char * name)
{
	logger->log("Reading scene descripion {}\n", name);
	std::ifstream in(name);
	if (!in.good()) {
		logger->log("unable to read from {}", name);
	}
	std::string s;
	while (!in.eof()) {
		auto c = in.get(); if (c != EOF) s += c;
	}
	Reader(this, s).read();
	auto node = makeNode("script");
	node->addAttribute("name", name);
	if (!currentSession)return;
	for (auto i : currentSession->findAllTags("script")) {
		if (i->getAttribute("name") == name)
			return;
	}
	currentSession->addNode(node);
}

void Miyuki::Scene::prepare()
{
	tracer.logger = pmRenderer.logger = logger;
	camera = dummy;
	sampleCount = 0;
	session.cnt = sampleCount;
	session.w = w; session.h = h;
	restore();
	srand((int)time(0));
	sampleCount = session.cnt;
	Xi = new Seed[w * h * 3];
	for (int i = 0; i < w*h; i++)
		Xi[3 * i + 2] = rand();
	int cnt = 0;
	for (auto i : objects) {
		i->id = cnt++;
		if (i->getMaterial().emittance.length() > 0.2) {
			lights.emplace_back(i);
		}
	}
	logger->log("Object count: {}\nLight count: {}\n", objects.size(),lights.size()); 
	logger->log("Building acceleration structure\n");
	accel = new BVH();
	accel->construct(objects);
	logger->log("{}", accel->getBuildInfo());
	tracer.prepare();
	pmRenderer.prepare();
	logger->log("Rendering session '{}'\n",session.name);
}

void Miyuki::Scene::renderOneSample()
{
	dummy = camera;
	parallelFor(0, w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < h; y++) {
			trace(x, y);
		}
	});
	sampleCount++;
}

void Miyuki::Scene::preview()
{
	dummy = camera;
	parallelFor(0, w, [&](unsigned int i) {
		int x = i;
		for (int y = 0; y < h; y++) {
			screen[x + w * y] *= sampleCount;
			screen[x + w * y] += tracer.raycast(x,y);
			screen[x + w * y] /= (1 + sampleCount);
		}
	});
	sampleCount++;
}

void Miyuki::Scene::render(int N)
{
	
	for (int i = 0; i < N; i++) {
		if (i && i % 4 == 0) {
			saveSession();
		}
		auto start = std::chrono::system_clock::now();
		renderOneSample();
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		logger->log("Iteration: {} {} Samples/sec\r",
			sampleCount,
			(w*h) / elapsed_seconds.count());
		if(option.sleepTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(option.sleepTime));
	}
	saveSession();
}
struct Polygon {
	std::vector<vec3> vertices;
	std::vector<Primitive*> trigs;
	Material material;

	Polygon() {}

	void addVertex(const vec3 &vector) {
		vertices.emplace_back(vector);
	}

	const std::vector<Primitive*> &getTrigs() {
		if (trigs.empty() && vertices.size() >= 3) {
			for (int i = 1; i < vertices.size() - 1; i++) {
				Triangle* triangle = new Triangle(vertices[0], vertices[i], vertices[i + 1], material);
				trigs.emplace_back(triangle);
			}
		}
		return trigs;
	}
};

template<>
struct fmt::formatter<vec3> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const vec3 &v, FormatContext &ctx) {
		return format_to(ctx.begin(), "({}, {}, {})", v.x(), v.y(), v.z());
	}
};

template<>
struct fmt::formatter<AABB> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const AABB &box, FormatContext &ctx) {
		return format_to(ctx.begin(), "[{}, {}]", box.min, box.max);
	}
};


void Miyuki::Scene::initLogger()
{
	logger = new Logger();
}

Miyuki::Scene::Scene(int _w, int _h) :w(_w), h(_h),tracer(this),pmRenderer(this) {
	sessionFilename = "Miyuki.session";
	screen.resize(w* h);
	option.sleepTime = 0;
	worldColor = vec3(0,0,0);

}

void  Miyuki::Scene::loadObj(const char *filename, vec3 translation, vec3 rotation,Float scale) {
	logger->log("Loading {0}\n", filename);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);

	if (!err.empty()) { // `err` may contain warning message.
		logger->log( "{}\n", err);
	}

	if (!ret) {
		logger->log( "cannot open file {}", filename);
		return;
	}
	logger->log("Adding objects...\n");
	std::vector<Polygon> polygons;
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			auto id = shapes[s].mesh.material_ids[f];
			Material material;
			if (id >= 0 && id < materials.size()) {
				material.Ni = materials[id].ior;
				material.Tr = 1 - materials[id].dissolve;
				material.emittance.x() = materials[id].emission[0];
				material.diffuse.x() = materials[id].diffuse[0];
				material.specular.x() = materials[id].specular[0];
				material.emittance.y() = materials[id].emission[1];
				material.diffuse.y() = materials[id].diffuse[1];
				material.specular.y() = materials[id].specular[1];
				material.emittance.z() = materials[id].emission[2];
				material.diffuse.z() = materials[id].diffuse[2];
				material.specular.z() = materials[id].specular[2];
				material.emissionStrength = material.emittance.max();
				if(material.emissionStrength>eps)
					material.emittance /= material.emissionStrength;
			}
			else {
				material.emittance.x() = 0;
				material.diffuse.x() = 1;
				material.specular.x() = 0;
				material.emittance.y() = 0;
				material.diffuse.y() = 1;
				material.specular.y() = 0;
				material.emittance.z() = 0;
				material.diffuse.z() = 1;
				material.specular.z() = 0;
				material.emissionStrength = 0;
			}
			//logger->log("{} {} {}\n", material.emittance, material.diffuse, material.specular);
			Polygon polygon;
			polygon.material = material;
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				/*tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];*/
				vec3 vert{ vx, vy, vz };
				vert = rotate(vert, vec3(1, 0, 0), rotation.x());
				vert = rotate(vert, vec3(0, 1, 0), rotation.y());
				vert = rotate(vert, vec3(0, 0, 1), rotation.z());
				vert *= scale;
				vert += translation;
				polygon.addVertex(vert);
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
RenderContext Miyuki::Scene::getRenderContext(int x0, int y0)
{
	Seed * _Xi = &Xi[x0 + y0 * w];
	Float x = (2 * (Float)x0 / w - 1)* static_cast<Float>(w) / h;
	Float y = 2 * (1 - (Float)y0 / h) - 1;
	vec3 ro = camera.pos;
	Float dx = 2.0 / h, dy = 2.0 / h;
	vec3 jitter = vec3(dx*erand48(_Xi), dy*erand48(_Xi), 0);
	Float z = 2.0 / tan(camera.aov / 2);
	vec3 rd = vec3(x, y, 0) + jitter - vec3(0, 0, -z);
	rd.normalize();
	rd = rotate(rd, vec3(1, 0, 0),camera.dir.y()); 
	rd = rotate(rd, vec3(0, 1, 0), camera.dir.x());
	RenderContext ctx(this, Ray(ro, rd), _Xi);
	ctx.directLighting = option.directLighting;
	return ctx;
}
Scene::~Scene()
{
	delete[] Xi;
}

void Miyuki::Scene::Reader::read()
{
	tokens.clear();
	p = 0;
	while (cur()) {
		while (cur() && isspace(cur()))next();
		if (cur() == '#') {
			while (cur() && cur() != '\n')next();
		}
		else {
			std::string s;
			while (cur() && !isspace(cur())) {
				s += cur(); next();
			}
			tokens.emplace_back(s);
		}
	}
	int i = 0;
	auto tok = [&](unsigned int x)->const std::string {
		return x < tokens.size() ? tokens[x] : "";
	};
	auto parseBool = [&]()->bool {
		auto t = tok(i++);
		if (t == "true")return true;
		if (t == "false")return false;
		scene->logger->log( "unaccepted bool value {}, defaulting to false\n", t);
		return false;
	};
	auto parseFloat = [&]()->Float {
		return std::stod(tok(i++));
	};
	auto parseVec = [&]()->vec3 {
		if (tok(i) != "v") {
			scene->logger->log( "vector expected\n");
			return vec3();
		}
		i++;
		try {
			auto x = std::stod(tok(i++));
			auto y = std::stod(tok(i++));
			auto z = std::stod(tok(i++));
			return vec3(x, y, z);
		}
		catch (std::invalid_argument & e) {
			return vec3();
		}
	};
	
	auto parseMaterial = [&]()->Material {
		if (tok(i) != "mtl") {
			scene->logger->log( "mtl expected\n");
			return Material();
		}
		i++;
		try {
			auto Ka = parseVec();
			auto Kd = parseVec();
			auto Ks = parseVec();
			return Material(Ka,Kd,Ks);
		}
		catch (std::invalid_argument & e) {
			return Material();
		}
	};
	auto parseSphere = [&]()->Primitive* {
		if (tok(i) != "sphere") {
			scene->logger->log( "sphere expected\n");
			return nullptr;
		}
		i++;
		try {
			auto center = parseVec();
			auto r = std::stod(tok(i++));
			auto m = parseMaterial();
			return new Sphere(center, r, m);
		}
		catch (std::invalid_argument & e) {
			return nullptr;
		}
	};
	auto parseLoad = [&]() {
		if (tok(i) != "load") {
			scene->logger->log("load expected\n");
			return nullptr;
		}
		i++;
		auto name = tok(i++);
		std::string s;
		std::istringstream in(name);
		in >> s;
		scene->loadObj(s.c_str());
	};
	auto parseCamPos = [&]() {
		if (tok(i) != "cam") {
			scene->logger->log( "cam expected\n");
		}
		i++;
		auto v = parseVec();
		scene->setCameraPos(v.x(), v.y(), v.z());
	};
	auto parseSet = [&]() {
		if (tok(i) != "set") {
			scene->logger->log( "set expected\n");
		}
		i++;
		auto opt = tok(i++);
		if (opt == "startNew") {
			scene->option.startNew = parseBool();
			scene->logger->log("set startNew to {}\n", scene->option.startNew);
		}
		else if (opt == "maxDepth") {
			scene->option.maxDepth = std::stod(tok(i++));
			scene->logger->log("set maxDepth to {}\n", scene->option.maxDepth);
		}
		else if (opt == "directLighting") {
			scene->option.directLighting = parseBool();
			scene->logger->log("set directLighting to {}\n", scene->option.directLighting);
		}
		else if (opt == "sleepTime") {
			scene->option.sleepTime = std::stod(tok(i++));
			scene->logger->log("set sleepTime to {}\n", scene->option.sleepTime);
		}
		else {
			scene->logger->log( "unrecognized option {}\n", opt);
		}
	};
	while (i < tokens.size()) {
		auto t = tok(i);
		if (tok(i) == "sphere") {
			scene->addObject(parseSphere());
		}
		else if (tok(i) == "load") {
			parseLoad();
		}
		else if (tok(i) == "cam") {
			parseCamPos();
		}
		else if (tok(i) == "set") {
			parseSet();
		}
		else {
			scene->logger->log("unrecognized instruction {}. Skipping\n", tok(i));
			i++;
		}
	}
}
