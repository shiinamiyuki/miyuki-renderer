#ifndef MIYUKI_GRAPHDETAIL_H
#define MIYUKI_GRAPHDETAIL_H

#include <miyuki.h>
#include <utils/noncopymovable.hpp>
#include <graph/node.h>
#include <graph/graphroot.h>

namespace Miyuki {
	namespace Graph {

		class Graph {
			std::random_device rd;
			std::uniform_int_distribution<uint64_t> dist;
			std::vector<Node*> _nodeList;
			Graph::Root* root = nullptr;

			// owns the nodes
			std::unordered_map<std::string, std::unique_ptr<Node>> _allNodes;

			std::unordered_map<std::string, std::unique_ptr<IDeserializer>> _deserializers;

			void addDefaultDeserializers();
			Graph();
			void deserialize(const json&);
		public:
			void bindRoot();
			template<class SequenceT>
			std::string generateUniqueName(const SequenceT& set) {
				std::string s;
				do {
					auto i = dist(rd);
					s = fmt::format("{:x}", i);
				} while (_allNodes.find(s) != _allNodes.end()
					|| set.find(s) != set.end());
				return std::string("#").append(s);
			}
			std::string generateUniqueName();
			bool addNode(std::unique_ptr<Node>);
			Node* getByName(const std::string & name);
			void serialize(json&);
			static std::unique_ptr<Graph> CreateGraph(const json&);
			static std::unique_ptr<Graph> NewGraph();
			void registerDeserializer(const std::string&, std::unique_ptr<IDeserializer>);
		};
	}
}

#endif