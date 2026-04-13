#include "password_util.hpp"
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#else
#include <openssl/sha.h>
#endif

namespace PasswordUtil {

std::string bytesToHex(const unsigned char* bytes, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[i];
    }
    return oss.str();
}

std::string hash(const std::string& password, const std::string& salt) {
    std::string combined = salt + password;
    unsigned char digest[32];

#ifdef __APPLE__
    CC_SHA256(combined.data(), static_cast<CC_LONG>(combined.size()), digest);
#else
    SHA256(reinterpret_cast<const unsigned char*>(combined.data()),
           combined.size(), digest);
#endif

    return bytesToHex(digest, 32);
}

std::string generateSalt() {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    std::string salt;
    salt.reserve(16);
    for (int i = 0; i < 16; ++i) {
        salt += chars[dis(gen)];
    }
    return salt;
}

bool verify(const std::string& password, const std::string& salt, const std::string& storedHash) {
    return hash(password, salt) == storedHash;
}

}
