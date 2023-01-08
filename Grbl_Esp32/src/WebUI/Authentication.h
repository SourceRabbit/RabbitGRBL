#pragma once

namespace WebUI
{
    // Authentication level
    enum class AuthenticationLevel : uint8_t
    {
        LEVEL_GUEST = 0,
        LEVEL_USER = 1,
        LEVEL_ADMIN = 2
    };

}
