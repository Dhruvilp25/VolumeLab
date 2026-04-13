#pragma once
#include <string>
#include <vector>
#include "database.hpp"

//Author: Negar Dheghaneian

// Program struct - represents a workout program
struct Program {
    int programId = 0;
    int userId = 0;
    std::string name;
    std::string createdAt;
};

// Repository class - handles database operations for programs
class ProgramRepository {
public:
    explicit ProgramRepository(Database& db) : db_(db) {}

    // Insert a new program, returns the new program ID
    int insert(int userId, const std::string& name);

    // Find a program by ID
    Program findById(int programId);

    // Get all programs for a user
    std::vector<Program> findByUserId(int userId);

    // Check if program name exists for user
    bool existsByName(int userId, const std::string& name);

    // Update program name
    bool update(int programId, const std::string& newName);

    // Delete a program
    bool deleteById(int programId);

    // Check if name exists, excluding a specific program
    bool existsByNameExcluding(int userId, const std::string& name, int excludeProgramId);

private:
    Database& db_;
};

// Service class - handles business logic for programs
class ProgramService {
public:
    explicit ProgramService(ProgramRepository& repo) : repo_(repo) {}

    // Create a new program
    Program createProgram(int userId, const std::string& name);

    // Get all programs for a user
    std::vector<Program> getProgramsByUser(int userId);

    // Get a program by ID
    Program getProgramById(int programId);

    // Rename a program
    bool renameProgram(int programId, const std::string& newName);

    // Delete a program (cascades to template days)
    bool deleteProgram(int programId);

private:
    ProgramRepository& repo_;
    bool isValidName(const std::string& name);
    std::string trim(const std::string& str);
};
