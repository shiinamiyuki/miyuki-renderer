#include <graph/graphdetail.h>
#include <graph/leaf.h>
#include <graph/materials.h>
#include <graph/meshes.h>
#include <utils/log.h>

namespace Miyuki {
	namespace Graph {
		Graph::Graph() {
		}
		Node* Graph::getByName(const std::string& name) {
			auto iter = _allNodes.find(name);
			if (iter != _allNodes.end())return iter->second.get();
			return nullptr;
		}
		bool Graph::addNode(std::unique_ptr<Node> node) {
			const auto& name = node->name();
			if (_allNodes.find(name) != _allNodes.end()) {
				return false;
			}
			_allNodes[name] = std::move(node);
			return true;
		}

		void Graph::serialize(json & j) {
			j = json::object();
			for (const auto& i : _allNodes) {
				j[i.first] = json::object();
				i.second->serialize(j[i.first]);
			}
		}
		void Graph::deserialize(const json & j) {
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

		void Graph::addDefaultDeserializers() {
			registerDeserializer("Int", std::make_unique<LeafNodeDeserializer<int>>());
			registerDeserializer("Float", std::make_unique<LeafNodeDeserializer<Float>>());
			registerDeserializer("Float3", std::make_unique<LeafNodeDeserializer<Vec3f>>());
			registerDeserializer("Transform", std::make_unique<LeafNodeDeserializer<Transform>>());
			//registerDeserializer("Image", std::make_unique<LeafNodeDeserializer<IO::ImagePtr>>());
			registerDeserializer("String", std::make_unique<LeafNodeDeserializer<std::string>>());
		}
		std::unique_ptr<Graph> Graph::CreateGraph(const json & j) {
			std::unique_ptr<Graph> graph(new Graph());
			graph->addDefaultDeserializers();
			graph->deserialize(j);
			return std::move(graph);
		}

		std::unique_ptr<Graph> Graph::NewGraph() {
			std::unique_ptr<Graph> graph(new Graph());
			graph->addDefaultDeserializers();
			std::unique_ptr<Meshes> meshes(new Meshes(graph.get()));
			std::unique_ptr<Materials> materials(new Materials(graph.get()));
			graph->addNode(std::move(materials));
			graph->addNode(std::move(meshes));
			return std::move(graph);
		}

		void Graph::registerDeserializer(const std::string & type, std::unique_ptr<IDeserializer> d) {
			_deserializers[type] = std::move(d);
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
				for (auto _i : subnodes()) {
					auto& i = _i->get();
					auto sub = json::object();
					sub["key"] = i.name;
					if (!i.to)
						sub["value"] = "";
					else {
						if (i.to->isLeaf()) {
							sub["value"] = json::object();
							i.to->serialize(sub["value"]);
						}
						else {
							sub["value"] = i.to->name();
						}
					}
					j["subnodes"].push_back(sub);
				}
			}
		}
	}
}