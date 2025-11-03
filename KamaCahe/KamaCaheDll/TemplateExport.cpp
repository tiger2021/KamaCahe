#include "pch.h"
#include "LRUCache.h"
#include "KLRUCache.h"
#include "KHashLRUCache.h"
#include <string>

#include "ExportKamaCacheDll.h"

// 显式实例化模板类
//模板（template<class K, class V>）的本质是代码生成器，不是普通类。
//编译器只有在使用到特定类型组合时才会生成对应代码。
template class KAMACACHE_API LRUCache<int, std::string>;
template class KAMACACHE_API KLRUCache<int, std::string>;
template class KAMACACHE_API KHashLRUCache<int, std::string>;