#pragma once
#include <string>
#include <vector>
#include "database.hpp"

struct Exercise {
    int exerciseId = 0;
    int userId = 0;
    std::string name;
    int isArchived = 0;
    std::string createdAt;
};

class ExerciseRepository {
public:
    explicit ExerciseRepository(Database& db) : db_(db) {}

    int insert(int userId, const std::string& name);
    Exercise findById(int exerciseId);
    Exercise findByUserAndName(int userId, const std::string& name);
    std::vector<Exercise> findByUserId(int userId);
    bool existsByUserAndName(int userId, const std::string& name);
    int findOrCreate(int userId, const std::string& name);

private:
    Database& db_;
};
