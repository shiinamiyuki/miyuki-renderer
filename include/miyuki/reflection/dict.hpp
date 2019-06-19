#pragma once

#include "object.hpp"
#include "primitive.hpp"

namespace Miyuki {
	namespace Reflection {
		// Only supports std::string -> T right now
		template<class K, class V>
		class Dict final : public Object {
			using Kty = typename _ConvertToPrimitiveType<K>::type;
			using Vty = typename _ConvertToPrimitiveType<V>::type;
			struct Pair {
				Kty* key;
				Vty* val;
			};
			constexpr static int maxListLength = 4;
			struct Bucket {
				std::list<Pair> list;
				bool insert(Object* key, Object* val) {
					for (auto& i : list) {
						if (i.key->equals(val)) {
							i.val = val;
							return false;;
						}
					}
					list.push_back(Pair{ key,val });
					if (list.size() > maxListLength)
						return true;
					return false;
				}
				void erase(Object* key) {
					for (auto iter = list.begin(); iter != list.end();iter++) {
						auto& i = *iter;
						if (i.key->equals(val)) {
							list.erase(iter);
							return;
						}
					}
				}
			};		
			using BucketArray = std::vector<Bucket>;
			BucketArray _buckets;
			std::unordered_map<UUID, Kty*> _map;
			bool insert(BucketArray& arr, Object * key, Object * val) {
				auto& bucket = getBucket(arr, key);
				return bucket.insert(key, val);
			}
			bool insert(BucketArray& arr, Pair pair) {
				return insert(arr, pair.key, pair.val);
			}

			private void rehash(size_t nBucket) {
				BucketArray _newBuckets(nBuckets);
				for (const auto &bucket : _buckets) {
					for (const auto& pair : bucket.list) {
						insert(_newBuckets, pair);
					}
				}
				std::swap(_newBuckets, _buckets);
			}
			Bucket& getBucket(BucketArray& arr, Object* key) {
				size_t idx = getHash(key) % _buckets.size();
				auto& bucket = arr[idx];
				return bucket;
			}
			const Bucket& getBucket(const BucketArray& arr, Object* key) {
				size_t idx = getHash(key) % _buckets.size();
				const auto& bucket = arr[idx];
				return bucket;
			}
		public:
			size_t size()const {
				size_t s = 0;
				for (const auto& b : _buckets) {
					s += b.list.size();
				}
				return s;
			}
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					auto s = new std::string("Miyuki::Reflection::Dict<");
					*s += Kty::__classinfo__()->name() +", ";
					*s += Vty::__classinfo__()->name();
					*s += ">";
					info->_name = s->c_str();
					o->classInfo.size = sizeof(Dict<T>);
					info->classInfo.base = Object::__classinfo__();
					info->classInfo.ctor = [=](const UUID& n) {return new Dict<T>(n); };
				});
				return info;
			}

			size_t getHash(Object* key) {
				return key->hashCode();
			}

			void insert(Object* key, Object* val) {
				if (insert(_buckets, key, val))
					rehash();
			}

			void insert(Kty* key, Vty* val) {
				insert((Object*)key, (Object*)val);
			}

			void erase(Kty* key, Vty* val) {
				auto& bucket = getBucket(_buckets, key);
				bucket.erase(key);
			}

			virtual std::vector<Object::Reference> getReferences()override {
				auto result = Object::getReferences();
				for (const auto& b : _buckets) {
					for (auto& pair : b.list) {
						auto* p = &pair;
						result.emplace_back([=](Object* o)->void {
							p->key = o;
						}, pair.key);
						result.emplace_back([=](Object* o)->void {
							p->val = o;
						}, pair.val);
					}
				}
				return result;
			}

			void deserialize(const json& j, const Resolver& resolve)override {
				Object::deserialize(j, resolve);
				_buckets.clear();
				_buckets.reserve(j["buckets"].size());
				for (const auto& i : j["buckets"]) {
					const auto& key = i["key"];
					const auto& val = i["val"];
					_buckets.push_back(Bucket());
					auto& bucket = _buckets.back();
					if (auto k = resolve(key)) {
						if (auto v = resolve(val)) {
							bucket.list.push_back(Pair(k.value(), v.value()));
						}
					}
				}
			}

			void serialize(json& j, SerializationState& state)const override {
				Object::serialize(j, state);
				j["buckets"] = json::array();
				for (const auto& b : _buckets) {
					json bucket = json::array();
					for (const auto pair : b.list) {
						json key, val;
						pair.key->serialize(key, state);
						pair.val->serialize(val, state);
						json pair;
						pair["key"] = key;
						pair["val"] = val;
						bucket.push_back(pair);
					}
					j["buckets"].push_back(bucket);
				}
			}
		};
	}
}