//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#ifndef SERIALFORGE_COMMON_H
#define SERIALFORGE_COMMON_H
#include <string>


namespace serialforge
{
    enum class OsType
    {
        MAC_OS,
        LINUX_OS,
        NOT_SUPPORTED_OS,
    };

    OsType get_os();
}

#endif //SERIALFORGE_COMMON_H
