#pragma once
#include <string>

namespace PasswordUtil {
    std::string hash(const std::string& password, const std::string& salt);
    std::string generateSalt();
    bool verify(const std::string& password, const std::string& salt, const std::string& storedHash);
}
