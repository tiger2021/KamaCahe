#pragma once

#ifdef KAMACACHE_EXPORTS
#define KAMACACHE_API __declspec(dllexport)
#else
#define KAMACACHE_API __declspec(dllimport)
#endif