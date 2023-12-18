// ElevatedPrivilegesChecker.cpp
#include "ElevatedPrivilegesChecker.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

bool ElevatedPrivilegesChecker::IsElevated() {
#ifdef _WIN32
    // Windows implementation
    return OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, nullptr) != FALSE;
#else
    // Linux implementation
    return geteuid() == 0;
#endif
}
