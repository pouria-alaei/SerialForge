//
// Created by Pooria Alaei on 8/6/2026 AD.
//
#include "../inc/common.h"
#include <iostream>
using namespace std;
namespace serialforge
{
    OsType get_os()
    {
#if defined(__APPLE__) || defined(__MAC__)
        constexpr OsType os_info = OsType::MAC_OS;
        return os_info;
#elif defined(__linux__)
        constexpr OsType os_info = OsType::LINUX_OS;
        return os_info;
#else
        constexpr constexpr OsType os_info = OsType::NOT_SUPPORTED_OS;
        return os_info;
#endif
    }
}
