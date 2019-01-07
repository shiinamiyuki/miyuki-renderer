#pragma once
#include "util.h"
#include "Logger.h"
#include "CoreCL.h"
#include "Accel.h"
#include "kernel/trace.h"
inline QVector3D fromVec3(const Vector&v) {
	return QVector3D(v.x, v.y, v.z);
}
inline Vector toVec3(const QVector3D&v) {
	return vec3(v.x(), v.y(), v.z());
}
class MainWindow;
class Render
{
	friend class MainWindow;
	std::vector<Primitive>	objects; 
	std::vector<Material>	allMaterials;
	std::unordered_map<std::string, int> materialMap;
	std::vector<int> lights;
	int width, height;
	int sampleCount;
	CoreCL::Context context;
	//buffers
	struct Buffer {
		CoreCL::Buffer Xi, renderCtx, objects, lights;
		CoreCL::Buffer materials;
		CoreCL::Buffer nodes, primPool;
		CoreCL::Buffer image;
		CoreCL::Buffer opt;
	};
	Buffer buffer;
	class Integrator {
	public:
		enum class Type {
			raycast,
			pt,
			bdpt,
			ao
		};		
	private:
		Type type;
		CoreCL::Kernel raycast;
		CoreCL::Kernel pt;
		CoreCL::Kernel bdpt;
		CoreCL::Kernel ao;
	public:
		CoreCL::Kernel& curIntegrator() {
			if (type == Type::pt) { return pt; }
			if (type == Type::bdpt) { return bdpt; }
			if (type == Type::ao) { return ao; }
			if (type == Type::raycast) { return raycast; }
		}
		
		void setContext(CoreCL::Context*ctx);
		void load();
		Integrator() { type = Type::raycast; }
		bool isAO()const { return type == Type::ao; }
		bool isPT()const { return type == Type::pt; }
		bool isBDPT()const { return type == Type::bdpt; }
		bool isRaycast()const { return type == Type::raycast; }
		void setIntegrator(Type t) { type = t; }
	}integrator;

	Vector pos, dir;
	Pixel * image;
	unsigned char * pixelBuffer;
	Seed * Xi;
	Logger * logger;
	BVH * bvhTree;
	int frame;
	// allocate CL buffer, auto release if reallocating
	void allocateBuffer();
	void setArgs();
	void initContext();
	void finishSetup();
	
	size_t globalSize, localSize;
	void renderPass();
public:
	int mode;
	struct Signal {
		volatile bool hasNew,alive;
	}signal;
	QVector3D getCameraDir()const { return fromVec3(dir); }
	void moveCameraBy(const QVector3D&);
	void moveCameraTo(const QVector3D&);
	void setCameraDir(const QVector3D&);
	void loadObj(const char *filename);
	void addObject(const Primitive&p) { objects.emplace_back(p); }
	void render();
	MainWindow * window;
	Render(MainWindow *);
	void copyImage(QPixmap& pixmap);
	void resetSample();
	void addMaterial(const std::string& s, const Material& m) {
		materialMap[s] = allMaterials.size();
		allMaterials.emplace_back(m);
		
	}
	int getMaterial(const std::string&s) {
		return materialMap[s];
	}
	~Render();
};
Material makeEmission(const Vector&);
Material makeDiffuse(const Vector&);
Material makeSpec(const Vector&,float r = 0);
Material makeRefr(const Vector&,float);
Primitive makeSphere(const Vector&,float,int m);