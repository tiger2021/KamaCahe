#pragma once
#include "LRUCache.h"
#include "ExportKamaCacheDll.h"
#include <unordered_map>
#include <memory>

template <typename Key, typename Value>
class KAMACACHE_API KLRUCache : public LRUCache<Key, Value>
{
public:
    KLRUCache(int capacity, int historyListCapacity, int k)
        : LRUCache<Key, Value>(capacity)
        , m_historyList(std::make_unique<LRUCache<Key, size_t>>(historyListCapacity))
        , m_k(k)
    {
    }

    virtual ~KLRUCache() noexcept override = default;

    Value get(Key key) override {
        Value value{};

        // 先尝试从主缓存获取
        bool inMainCache = LRUCache<Key, Value>::get(key, value);
        size_t historyCount = m_historyList->get(key);
        ++historyCount;
        m_historyList->put(key, historyCount);  // 更新访问次数到历史记录

        if (inMainCache) {
            return value;
        }

        if (historyCount >= m_k) {
            auto it = m_historyValueMap.find(key);
            if (it != m_historyValueMap.end()) {
                Value storeValue = it->second;
                m_historyList->remove(key);
                m_historyValueMap.erase(it);

                // 添加到主缓存
                LRUCache<Key, Value>::put(key, storeValue);
                return storeValue;
            }
        }
        return value;
    }

    void put(Key key, Value value) {
        Value existedValue{};
        bool inMainCache = LRUCache<Key, Value>::get(key, existedValue);
        if (inMainCache) {
            LRUCache<Key, Value>::put(key, value);
            return;
        }
        size_t historyCount = m_historyList->get(key);
        ++historyCount;
        m_historyList->put(key, historyCount);
        m_historyValueMap[key] = value;

        if (historyCount >= m_k) {
            m_historyList->remove(key);
            m_historyValueMap.erase(key);
            LRUCache<Key, Value>::put(key, value);
        }
    }

private:
    int m_k;  // 进入缓存队列的最少访问次数
    std::unique_ptr<LRUCache<Key, size_t>> m_historyList; // 访问数据历史记录
    std::unordered_map<Key, Value> m_historyValueMap;     // 存储未达到K次访问的数据值
    int m_test;
};
