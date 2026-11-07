#pragma once

#include <cstdio>
#include <cstring>
#include <functional>

// The project's own test harness: a failed CHECK prints where and goes on.
inline int failures = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            ++failures;                                                   \
        }                                                                 \
    } while (0)

// `child_tests NAME...` runs only the cases whose name contains one of the arguments.
inline int filter_count = 0;
inline char** filters = nullptr;

inline void run(const char* name, const std::function<void()>& fn) {
    bool wanted = filter_count == 0;
    for (int i = 0; i < filter_count && !wanted; ++i) wanted = std::strstr(name, filters[i]) != nullptr;
    if (!wanted) return;
    const int before = failures;
    std::printf("%s\n", name);
    fn();
    std::printf("  %s\n", failures == before ? "ok" : "FAILED");
}

// Cases of the integrated brain, in brain_tests.cpp.
void run_brain_tests();
