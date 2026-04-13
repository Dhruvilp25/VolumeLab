#pragma once
#include <string>
#include "database.hpp"

struct User {
    int userId = 0;
    std::string username;
    std::string passwordHash;
    std::string salt;
    std::string createdAt;
};

class UserRepository {
public:
    explicit UserRepository(Database& db) : db_(db) {}

    int insert(const std::string& username, const std::string& passwordHash, const std::string& salt);
    User findByUsername(const std::string& username);
    User findById(int userId);
    bool existsByUsername(const std::string& username);
    bool deleteById(int userId);

private:
    Database& db_;
};
