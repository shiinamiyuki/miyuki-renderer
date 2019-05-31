#include <graph/graphdetail.h>
#include <graph/leaf.h>
#include <utils/log.h>

/*
Sample json:
{
	"scene":{
		"type":"scene",
		"version": ....,
		"subnodes":[
			"materials",
			"meshes",
			"integrator",
			"camera",
			...
		]
	},
	"materials":{
		"type":"MaterialsRoot",
		"subnodes":[
		...
		]
	}
}

*/
namespace Miyuki {
	namespace Graph {
		Graph::Graph() {
		}
		bool Graph::addNode(const std::string& name,
			std::unique_ptr<Node> node) {
			if (_allNodes.find(name) != _allNodes.end()) {
				return false;
			}
			_allNodes[name] = std::move(node);
			return true;
		}

		void Graph::serialize(json& j) {
			j = json::object();
			for (const auto& i : _allNodes) {
				j[i.first] = json::object();
				i.second->serialize(j[i.first]);
			}
		}
		void Graph::deserialize(const json& j) {
			Assert(j.is_object());
			for (const auto& key : j.items()) {
				auto d = _deserializers[key.value().at("type").get<std::string>()].get();
				if (!d) {
					Log::log("Error deserializing object `{}`\n", key.key());
					continue;
				}
				auto node = d->deserialize(key.value(), this);
				_allNodes[node->name()] = std::move(std::unique_ptr<Node>(node));
			}
		}

		std::unique_ptr<Graph> Graph::CreateGraph(const json& j) {
			std::unique_ptr<Graph> graph(new Graph());
			graph->registerDeserializer("Int", std::make_unique<LeafNodeDeserializer<int>>());
			graph->registerDeserializer("Float", std::make_unique<LeafNodeDeserializer<Float>>());
			graph->registerDeserializer("Float3", std::make_unique<LeafNodeDeserializer<Vec3f>>());
			graph->registerDeserializer("Transform", std::make_unique<LeafNodeDeserializer<Transform>>());
			//graph->registerDeserializer("Image", std::make_unique<LeafNodeDeserializer<IO::ImagePtr>>());
			graph->registerDeserializer("String", std::make_unique<LeafNodeDeserializer<std::string>>());
			graph->deserialize(j);
			return std::move(graph);
		}

		void Graph::registerDeserializer(const std::string& type, std::unique_ptr<IDeserializer> d) {
			_deserializers[type] = std::move(d);
		}
		std::string Graph::generateUniqueName(const std::set<std::string>& set) {
			std::string s;
			do {
				auto i = dist(rd);
				s = fmt::format("{:x}", i);
			} while (_allNodes.find(s) != _allNodes.end()
				|| set.find(s) != set.end());
			return std::string("#").append(s);
		}
		std::string Graph::generateUniqueName() {
			std::string s;
			do {
				auto i = dist(rd);
				s = fmt::format("{:x}", i);
			} while (_allNodes.find(s) != _allNodes.end());
			return std::string("#").append(s);
		}

		void Node::serialize(json & j) const {
			j["name"] = _name;
			j["type"] = type();
			if (!isLeaf()) {
				j["subnodes"] = json::array();
				for (auto i : _subnodes) {
					j["subnodes"].push_back(i.name);
				}
			}
		}
	}
}