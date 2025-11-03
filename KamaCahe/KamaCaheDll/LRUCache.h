#pragma once
#include <iostream>
#include "KamaCahePolicy.h"
#include "LRUNode.h"
#include <unordered_map>
#include <mutex>
#include "ExportKamaCacheDll.h"

template <typename Key, typename Value>
class KAMACACHE_API LRUCache :public KamaCachePolicy<Key, Value>
{
public:
	// 类型别名 简化代码
	using LRUNodeType = LRUNode<Key, Value>;
	using NodePtr = std::shared_ptr<LRUNodeType>;
	using NodeMap = std::unordered_map<Key, NodePtr>;

	LRUCache(int capacity) :m_capacity(capacity) {
		initializeNode();
	};

	~LRUCache() {
		clear();
	};

	void clear() {
		// 断开并迭代释放，从头到尾避免递归析构
		auto node = m_dummyHead;
		while (node) {
			auto next = node->m_next;
			node->m_next.reset();    // 断开对下一个节点的拥有引用
			node = next;
		}
		m_dummyHead.reset();
		m_dummyTail.reset();
		m_cacheMap.clear();
	}

	void initializeNode() {
		m_dummyHead = std::make_shared<LRUNodeType>(Key(), Value());
		m_dummyTail = std::make_shared<LRUNodeType>(Key(), Value());
		m_dummyHead->m_next = m_dummyTail;
		m_dummyTail->m_pre = m_dummyHead;
	}

	void put(Key key, Value value) override {
		if (m_capacity <= 0) {
			std::cout << "Cache Capacity Must Greater Than 0!" << std::endl;
			return;
		}
		std::lock_guard<std::mutex> lock(m_mutex); // 自动加锁和解锁
		auto it = m_cacheMap.find(key);
		if (it != m_cacheMap.end()) {
			// 更新已有节点的值并移动到最近使用位置
			updateExistingNode(it->second, value);
		}else {
			//只有在put的时候才添加新节点
			addNewNode(key, value);
		}
	}

	bool get(Key key, Value& value) override {
		std::lock_guard<std::mutex> lock(m_mutex); // 自动加锁和解锁
		auto it = m_cacheMap.find(key);
		if (it != m_cacheMap.end()) {
			// 找到节点，获取值并移动到最近使用位置
			value = it->second->getValue();
			return moveToMostRecentlyUsed(it->second);
		}
		else
		{
			return false; // 未找到节点
		}
	}

	Value get(Key key) override {
		Value value{};
		bool found = get(key, value);
		return value;
	}

	bool remove(Key key) {
		std::lock_guard<std::mutex> lock(m_mutex); // 自动加锁和解锁
		auto it = m_cacheMap.find(key);
		if (it != m_cacheMap.end()) {
			return removeNode(it->second);
		}
		return false; // 未找到节点
	}

private:
	// 删除指定节点
	bool removeNode(NodePtr node) {
		if (!node) {
			return false;
		}
		auto preNode = node->m_pre.lock();
		auto nextNode = node->m_next;
		//std::lock_guard<std::mutex> lock(m_mutex);
		if (preNode && nextNode) {
			// 断开当前节点与前后节点的连接
			preNode->m_next = nextNode;
			nextNode->m_pre = preNode;
			// 从哈希表中删除该节点
			m_cacheMap.erase(node->getKey());
			return true;
		}
		return false;
	}

	// 更新已有节点的值并移动到最近使用位置
	bool updateExistingNode(NodePtr node, Value value) {
		if (!node) {
			return false;
		}
		//std::lock_guard<std::mutex> lock(m_mutex);
		node->setValue(value);
		return moveToMostRecentlyUsed(node);
	}

	// 将节点移动到最近使用位置
	bool moveToMostRecentlyUsed(NodePtr node) {
		if (!node) {
			return false;
		}
		auto preNode = node->m_pre.lock();
		auto nextNode = node->m_next;
		//std::lock_guard<std::mutex> lock(m_mutex);
		if (preNode && nextNode) {
			// 断开当前节点与前后节点的连接
			preNode->m_next = nextNode;
			nextNode->m_pre = preNode;
			// 将当前节点插入到尾部（最近使用位置）
			return insertNodeAtTail(node);
		}
		return false;
	}

	// 添加新节点
	bool addNewNode(const Key& key, const Value& value) {
		NodePtr newNode = std::make_shared<LRUNodeType>(key, value);
		if (!newNode) {
			std::cout << "Create New Node Failed!" << std::endl;
			return false;
		}
		if (m_cacheMap.size() >= m_capacity) {
			// 删除最久未使用的节点
			bool deleteFlag = deleteLRUNode();
			if (!deleteFlag) {
				std::cout << "Delete LRU Node Failed!" << std::endl;
				return false;
			}
		}
		// 插入到链表尾部（最近使用位置）
		bool insertFlag = insertNodeAtTail(newNode);
		if (!insertFlag) {
			std::cout << "Insert New Node at Tail Failed!" << std::endl;
			return false;
		}
		m_cacheMap[key] = newNode;
		return true;
	}

	// 将节点插入到链表尾部（最近使用位置）
	bool insertNodeAtTail(NodePtr node) {
		if (!node) {
			return false;
		}
		//std::lock_guard<std::mutex> lock(m_mutex);
		NodePtr tailPreNode = m_dummyTail->m_pre.lock();
		if (tailPreNode) {
			tailPreNode->m_next = node;
			node->m_pre = tailPreNode;
			node->m_next = m_dummyTail;
			m_dummyTail->m_pre = node;
			return true;
		}
		else {
			return false;
		}
	}

	// 删除最久未使用的节点
	bool deleteLRUNode() {
		auto lruNode = m_dummyHead->m_next;
		if (lruNode && lruNode != m_dummyTail) {
			auto lruKey = lruNode->getKey();
			// 断开头节点与下一个节点的连接
			auto nextNode = lruNode->m_next;
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