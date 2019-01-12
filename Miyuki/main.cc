#include "Scene.h"
#include "XMLReader.h"
#include "KDTree.h"
using namespace Miyuki;
template<>
struct fmt::formatter<vec3> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const vec3 &v, FormatContext &ctx) {
		return format_to(ctx.begin(), "({}, {}, {})", v.x(), v.y(), v.z());
	}
};
void testKDTree() {
	KDTree<vec3> tree;
	std::vector<vec3> pts;
	pts.reserve(1000 * 1000 * 4);
	std::random_device rd;
	std::uniform_real_distribution<Float> dist;	
	for (int i = 0; i < 1000000; i++) {
		pts.emplace_back(vec3(dist(rd), dist(rd), dist(rd))*500);
	}
	auto start = std::chrono::system_clock::now();
	tree.construct(pts,0,pts.size(),0);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	fmt::print("{}\n", elapsed_seconds.count());
	std::vector<vec3> v;
	auto p = vec3(0.5, 0.5, 0.5)*200;
	Float r = tree.knn(p, 30, 20, v);
	fmt::print("found {}, raidus = {}\n", v.size(),r);
	fmt::print("brute force knn\n");
	std::sort(pts.begin(), pts.end(), [&](const vec3&a, const vec3&b) {
		return (p - a).length() < (p - b).length();
	});
	fmt::print("checking results\n");
	std::sort(v.begin(), v.end(), [&](const vec3&a, const vec3&b) {
		return (p - a).length() < (p - b).length();
	});
	Float accum = 0;
	for (int i = 0; i < v.size(); i++) {
		accum += (v[i] - pts[i]).length();
		fmt::print("{} {}\n", v[i], pts[i]);
	}
	fmt::print("error {}\n", accum);
}
void defaultScene() {
	Scene scene(1000, 1000);
	scene.openSession("scene_test");
	scene.readScript("cornell.txt");
	scene.prepare();
	scene.render();
}
int main(int argc, char *argv[])
{
	MLTSampler s;
	Sampler sampler;
	Seed Xi[] = { 0,2,2 };
	sampler.Xi = Xi;
	sampler.s = &s;
	s.start();
	for (int i = 0; i < 10; i++) {
		fmt::print("{} ", sampler.sample());
	}
	fmt::print("\n");
	s.start();
	for (int i = 0; i < 10; i++) {
		fmt::print("{} ", sampler.sample());
	}
	fmt::print("\n");
	
	return 0;
}
