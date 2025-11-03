#pragma once
#include <vector>
#include "KLRUCache.h"
#include "ExportKamaCacheDll.h"

//KHashLRUCache 目标就是更适应高并发场景。
// 解决的关键在于尽可能减小临界区，通过引入 hash 分片机制，将原有的缓存结构拆分为多个独立的子缓存。
// 它通过哈希函数将缓存数据分布到 N 个LRU 分片上，而查找过程也先通过哈希函数定位到具体分片，再进行数据操作。
// 这样一来，大幅提升了并发访问时的读写并行度，显著降低了锁冲突带来的性能损耗。


template <typename Key, typename Value>
class KAMACACHE_API KHashLRUCache
{
public:
	KHashLRUCache(size_t capacity,int sliceNum)
		:m_capacity(capacity)
		,m_sliceNum(sliceNum>0?sliceNum:std::thread::hardware_concurrency())
	{
		//设置每个切片的容量
		size_t eachSliceCapacity = std::ceil((double)capacity /static_cast<double>( m_sliceNum));
		for (int i = 0; i < m_sliceNum; ++i) {
			m_lruSliceCachesVector.emplace_back(std::make_shared<KLRUCache<Key, Value>>(eachSliceCapacity, eachSliceCapacity, 2));
		}
	}
	void put(Key key, Value value) {
		size_t hashValue = Hash(key);
		int sliceIndex = static_cast<int>(hashValue % m_sliceNum);
		m_lruSliceCachesVector[sliceIndex]->put(key, value);
	}

	bool get(Key key, Value& value) {
		size_t hashValue = Hash(key);
		int sliceIndex = static_cast<int>(hashValue % m_sliceNum);
		return m_lruSliceCachesVector[sliceIndex]->get(key, value);
	}

	Value get(Key key) {
		Value value;

		size_t hashValue = Hash(key);
		int sliceIndex = static_cast<int>(hashValue % m_sliceNum);
		get(key, value);
		return value;
	}

	

	~KHashLRUCache() {}

private:
	size_t Hash(Key key) {
		std::hash<Key> hashFunc;
		// 根据key，返回哈希值
		return hashFunc(key);
	}

private:
	size_t m_capacity;      // 缓存容量
	int m_sliceNum;    // 切片数量
	std::vector<std::shared_ptr<KLRUCache<Key, Value>>> m_lruSliceCachesVector;

};


