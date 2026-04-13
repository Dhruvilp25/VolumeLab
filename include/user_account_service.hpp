#pragma once
#include <string>
#include "user_repository.hpp"

struct LoginResult {
    bool success = false;
    int userId = 0;
    std::string username;
    std::string errorMessage;
};

struct RegisterResult {
    bool success = false;
    int userId = 0;
    std::string username;
    std::string errorMessage;
};

class UserAccountService {
public:
    explicit UserAccountService(UserRepository& repo) : repo_(repo) {}

    RegisterResult registerUser(const std::string& username, const std::string& password);
    LoginResult login(const std::string& username, const std::string& password);
    bool deleteAccount(int userId);

private:
    UserRepository& repo_;
    static bool isValidPassword(const std::string& password);
    static std::string trim(const std::string& str);
};
