#include "pch.h"
#include "LRUCache.h"
#include "KLRUCache.h"
#include <string>

#include "ExportKamaCacheDll.h"

// 显式实例化模板类
template class KAMACACHE_API LRUCache<int, std::string>;
template class KAMACACHE_API KLRUCache<int, std::string>;