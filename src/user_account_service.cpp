#include "user_account_service.hpp"
#include "password_util.hpp"
#include <algorithm>
#include <cctype>

std::string UserAccountService::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool UserAccountService::isValidPassword(const std::string& password) {
    return password.size() >= 6;
}

RegisterResult UserAccountService::registerUser(const std::string& username, const std::string& password) {
    RegisterResult result;

    std::string uname = trim(username);
    if (uname.empty()) {
        result.errorMessage = "Username cannot be empty";
        return result;
    }
    if (password.empty()) {
        result.errorMessage = "Password cannot be empty";
        return result;
    }
    if (!isValidPassword(password)) {
        result.errorMessage = "Password must be at least 6 characters";
        return result;
    }
    if (repo_.existsByUsername(uname)) {
        result.errorMessage = "Username already taken";
        return result;
    }

    try {
        std::string salt = PasswordUtil::generateSalt();
        std::string hash = PasswordUtil::hash(password, salt);
        int userId = repo_.insert(uname, hash, salt);

        result.success = true;
        result.userId = userId;
        result.username = uname;
        return result;
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        return result;
    }
}

LoginResult UserAccountService::login(const std::string& username, const std::string& password) {
    LoginResult result;

    std::string uname = trim(username);
    if (uname.empty()) {
        result.errorMessage = "Username cannot be empty";
        return result;
    }
    if (password.empty()) {
        result.errorMessage = "Password cannot be empty";
        return result;
    }

    if (!repo_.existsByUsername(uname)) {
        result.errorMessage = "Invalid username or password";
        return result;
    }

    try {
        User u = repo_.findByUsername(uname);
        if (!PasswordUtil::verify(password, u.salt, u.passwordHash)) {
            result.errorMessage = "Invalid username or password";
            return result;
        }

        result.success = true;
        result.userId = u.userId;
        result.username = u.username;
        return result;
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        return result;
    }
}

bool UserAccountService::deleteAccount(int userId) {
    return repo_.deleteById(userId);
}
