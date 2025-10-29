#pragma once
#include <iostream>
#include "KamaCahePolicy.h"
#include "LRUNode.h"
#include <unordered_map>
#include <mutex>
	template <typename Key, typename Value>
	class KLRUCache :public KamaCachePolicy<Key, Value>
	{
	public:
		// 类型别名 简化代码
		using LRUNodeType = LRUNode<Key, Value>;
		using NodePtr = std::shared_ptr<LRUNodeType>;
		using NodeMap = std::unordered_map<Key, NodePtr>;

		KLRUCache(int capacity) :m_capacity(capacity) {
			initializeNode();
		};

		~KLRUCache() {};

		void initializeNode() {
			m_dummyHead = std::make_shared<LRUNodeType>(Key(),Value());
			m_dummyTail = std::make_shared<LRUNodeType>(Key(), Value());
			m_dummyHead->m_next = m_dummyTail;
			m_dummyTail->m_pre = m_dummyHead;
		}

		void put(Key key, Value value) override {

		}

		bool get(Key key, Value& value) override {

		}
	
		Value get(Key key) override {

		}

		bool updateExistingNode(NodePtr node, Value value) {
			if (!node) {
				return false;
			}
			node->setValue(value);
			return moveToMostRecentlyUsed(node);
		}

		bool moveToMostRecentlyUsed(NodePtr node) {
			if (!node) {
				return false;
			}
			auto preNode = node->m_pre.lock();
			auto nextNode = node->m_next.lock();
			if (preNode && nextNode) {
				// 断开当前节点与前后节点的连接
				preNode->m_next = nextNode;
				nextNode->m_pre = preNode;
				// 将当前节点插入到尾部（最近使用位置）
				auto tailPreNode = m_dummyTail->m_pre.lock();
				if (tailPreNode) {
					tailPreNode->m_next = node;
					node->m_pre = tailPreNode;
					node->m_next = m_dummyTail;
					m_dummyTail->m_pre = node;
				}
				return true;
			}
			return false;
		}

		bool addNewNode(const Key& key,const Value& value) {
			if(m_cacheMap.size()>=m_capacity){
				// 删除最久未使用的节点
				deleteLRUNode();
			}
		}

		bool deleteLRUNode() {
			auto lruNode = m_dummyHead->m_next.lock();
			if (lruNode && lruNode != m_dummyTail) {
				auto lruKey = lruNode->getKey();
				// 断开头节点与下一个节点的连接
				auto nextNode = lruNode->m_next.lock();
				if (nextNode) {
					m_dummyHead->m_next = nextNode;
					nextNode->m_pre = m_dummyHead;
				}
				// 从哈希表中删除该节点
				m_cacheMap.erase(lruKey);
				return true;
			}
			return false;
		}

	private:
		int m_capacity; // 缓存容量
		NodeMap m_cacheMap; // 哈希表存储缓存节点
		NodePtr m_dummyHead; // 双向链表头节点
		NodePtr m_dummyTail; // 双向链表尾节点
		std::mutex m_mutex; // 互斥锁，保护共享数据
	};
