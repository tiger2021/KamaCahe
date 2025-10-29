#pragma once

#include <iostream>

	template<typename Key, typename Value> 
	class LRUCache;
	template <typename Key, typename Value>
	class LRUNode
	{
	public:
		LRUNode() {};
		LRUNode(Key key, Value value) :m_key(key), m_value(value), m_accessCount(1)
		{};
		~LRUNode() {};

		Key getKey() const
		{
			return m_key;
		}

		Value getValue() const
		{
			return m_value;
		}

		void setValue(Value value)
		{
			m_value = value;
		}

		size_t getAccessCount() const
		{
			return m_accessCount;
		}

		void incrementAccessCount()
		{
			++m_accessCount;
		}

	private:
		Key	m_key;
		Value m_value;
		size_t m_accessCount;
		std::weak_ptr<LRUNode<Key, Value>> m_pre; //std::weak_ptr智能指针，防止循环引用
		std::shared_ptr<LRUNode<Key, Value>> m_next;

		//友元类 能够访问private成员,protected成员和public成员
		friend class KLRUCache<Key, Value>;
	};		