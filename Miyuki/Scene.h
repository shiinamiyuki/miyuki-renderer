#pragma once
#include "Miyuki.h"
#include "Primitive.h"
#include "Camera.h"
#include "Accel.h"
#include "XMLReader.h"
#include "Logger.h"
#include "PathTracer.h"
#include "PhotonMap.h"
#include "sppm.h"
#include "mlt.h"
namespace Miyuki {
	class PhotonMapRenderer;
	class Scene
	{
		friend class PathTracer;
		friend class PhotonMapRenderer;
		friend class SPPM;
		friend class BDPT;
		friend class MLT;
		SamplerBase * samplerBase;
	protected:
		Logger * logger;
		Camera camera, dummy;
		friend class Primitive;
		struct Session {
			int w, h;
			int cnt;
			std::string name;
		};
		struct Option {
			bool startNew;
			int maxDepth;
			int rrStartDepth;
			bool directLighting;
			int sleepTime;
			struct SPPMpara {
				Float initialRadius;
				Float alpha;
				int numPhoton;
				enum DirectLighting {
					sppmDirect,
					RTDirect
				}direct;
				SPPMpara() {
					initialRadius = 1;
					alpha = 0.7;
					numPhoton = 100000;
					direct = sppmDirect;
				}
			}sppm;
			Option() {
				startNew = false;
				maxDepth = 5;
				rrStartDepth = 0;
				directLighting = true;
				sleepTime = 0;

			}
		};
		enum class Mode {
			preview,
			renderPathTracing,
			renderPM,
			renderBDPT,
			renderMLT,
		}mode;
		struct Reader {
			vector<std::string> tokens;
			std::string source;
			int p;
			int get(unsigned int x) { return x < source.length() ? source[x] : 0; }
			int cur() { return get(p); }
			void next() { p++; }
			Reader(Scene *_s,const std::string&s) :scene(_s),source(s),p(0) {}
			void read();
			Scene * scene;
		};
		AccelerationStructure * accel;
		XMLNode sessionData;
		XMLNode currentSession;
		const char *sessionFilename;
	public:
		enum IntegratorType {
			none,
			pathTracing,
			photonMapping,
			stochasticPhotonMapping
		};
		Option option;
	protected:
		IntegratorType integratorType;
		Session session;
		vector<Primitive*> objects;
		vector<Primitive*> lights;
		vector<vec3> screen;
		Seed * Xi;
		vec3 worldColor;
		int w, h;
		int sampleCount;
		BDPT bdpt;
		PathTracer tracer;
		PhotonMapRenderer pmRenderer;
		SPPM sppm;
		MLT pssmlt;
		void trace(int x, int y);
		Ray cameraRay(int x, int y);
		void loadSessionData();
		void restore();
	public:
		void readScript(const char *name);
		void openSession(const char*name) {
			initLogger();
			session.name = name;
			loadSessionData();
		}
		void saveSession();
		void setCameraPovDegrees(Float x) {
			camera.aov = x / 180.0 * pi;
		}
		void setCameraPos(Float a, Float b, Float c) {
			camera.pos = vec3(a, b, c);
		}
		virtual void initLogger();
		Scene(int _w, int _h);
		struct Signal {
			volatile bool hasNew, alive, reset;
			Signal() :hasNew(false), alive(true), reset(false) {}
		}signal;
		void setSleep(int x) { option.sleepTime = x; }
		void loadObj(const char*filename, vec3 translation=vec3(0,0,0), vec3 rotation = vec3(0, 0, 0),Float scale=1);
		void addObject(Primitive * o) { if(o)objects.emplace_back(o); }
		void prepare();
		void renderOneSample();
		void preview();
		void render(int N = 1000000);
		void resetSample() { signal.reset = true; }
		vec3 getCameraDir()const { return camera.dir; }
		void setCameraDir(const vec3& v) { dummy.dir = v; }
		void moveCameraTo(const vec3& v) { dummy.pos = v; }
		void moveCameraBy(const vec3& v) { dummy.pos += v; }
		void setWorldColor(const vec3&v) { worldColor = v; }
		AccelerationStructure * getAccel()const { return accel; }
		vector<vec3>& getScreen() { return screen; }
		vec3 getDimension()const { return vec3(w, h, 0); }
		int getSampleCount()const { return sampleCount; }
		RenderContext getRenderContext(int x, int y);
		Logger * getLogger()const { return logger; }
		void render(Integrator *i) { i->render(this); sampleCount++; }
		void useIntegrator(IntegratorType ty) { integratorType = ty; }
		~Scene();
	};
}
