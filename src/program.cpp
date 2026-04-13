/**
 * @file program.cpp
 * @brief Implements ProgramRepository and ProgramService methods.
 *
 * Contains all SQLite queries for program CRUD and the service-layer
 * validation logic (name trimming, duplicate checking)
 *
 * @author Negar Dehghaneian
 */

#include "program.hpp"
#include <stdexcept>

/// @brief Insert a new program row and return the generated ID.
int ProgramRepository::insert(int userId, const std::string& name) {
    const std::string sql = "INSERT INTO programs (user_id, name) VALUES (?, ?);";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, name);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("insert program failed");
        }

        // Get the new program ID
        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        int programId = 0;
        if (sqlite3_step(idStmt) == SQLITE_ROW) {
            programId = Database::colInt(idStmt, 0);
        }
        db_.finalize(idStmt);
        db_.finalize(stmt);

        return programId;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Look up a program by primary key.
Program ProgramRepository::findById(int programId) {
    const std::string sql = R"SQL(
        SELECT program_id, user_id, name, created_at
        FROM programs
        WHERE program_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("Program not found");
        }

        Program p;
        p.programId = Database::colInt(stmt, 0);
        p.userId = Database::colInt(stmt, 1);
        p.name = Database::colText(stmt, 2);
        p.createdAt = Database::colText(stmt, 3);

        db_.finalize(stmt);
        return p;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Retrieve all programs for a user, newest first.
std::vector<Program> ProgramRepository::findByUserId(int userId) {
    const std::string sql = R"SQL(
        SELECT program_id, user_id, name, created_at
        FROM programs
        WHERE user_id = ?
        ORDER BY created_at DESC;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);

        std::vector<Program> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("findByUserId: sqlite3_step failed");
            }

            Program p;
            p.programId = Database::colInt(stmt, 0);
            p.userId = Database::colInt(stmt, 1);
            p.name = Database::colText(stmt, 2);
            p.createdAt = Database::colText(stmt, 3);
            out.push_back(p);
        }

        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Return true if the given program name exists for the user.
bool ProgramRepository::existsByName(int userId, const std::string& name) {
    const std::string sql = R"SQL(
        SELECT COUNT(*) FROM programs
        WHERE user_id = ? AND name = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, name);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = Database::colInt(stmt, 0);
        }

        db_.finalize(stmt);
        return count > 0;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Update a program's name in the database.
bool ProgramRepository::update(int programId, const std::string& newName) {
    const std::string sql = R"SQL(
        UPDATE programs SET name = ? WHERE program_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindText(stmt, 1, newName);
        Database::bindInt(stmt, 2, programId);

        int rc = sqlite3_step(stmt);
        db_.finalize(stmt);

        if (rc != SQLITE_DONE) {
            throw std::runtime_error("update program failed");
        }
        return true;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Delete a program row by its ID.
bool ProgramRepository::deleteById(int programId) {
    const std::string sql = R"SQL(
        DELETE FROM programs WHERE program_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);

        int rc = sqlite3_step(stmt);
        db_.finalize(stmt);

        if (rc != SQLITE_DONE) {
            throw std::runtime_error("delete program failed");
        }
        return true;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Check for duplicate name, excluding one program (used during rename).
bool ProgramRepository::existsByNameExcluding(int userId, const std::string& name, int excludeProgramId) {
    const std::string sql = R"SQL(
        SELECT COUNT(*) FROM programs
        WHERE user_id = ? AND name = ? AND program_id != ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, name);
        Database::bindInt(stmt, 3, excludeProgramId);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = Database::colInt(stmt, 0);
        }

        db_.finalize(stmt);
        return count > 0;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

// ── ProgramService - Business Logic ──────────────────────────────

/// @brief Remove leading/trailing whitespace.
std::string ProgramService::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

/// @brief Return true if the trimmed name is non-empty.
bool ProgramService::isValidName(const std::string& name) {
    std::string trimmed = trim(name);
    return !trimmed.empty();
}

/// @brief Create a program after validating and deduplicating the name.
Program ProgramService::createProgram(int userId, const std::string& name) {
    // Validate name
    std::string trimmedName = trim(name);
    if (trimmedName.empty()) {
        throw std::invalid_argument("Program name cannot be empty");
    }

    // Check for duplicate name
    if (repo_.existsByName(userId, trimmedName)) {
        throw std::runtime_error("Program name already exists");
    }

    // Insert and return the created program
    int programId = repo_.insert(userId, trimmedName);
    return repo_.findById(programId);
}

/// @brief Delegate to repository to list all programs for a user.
std::vector<Program> ProgramService::getProgramsByUser(int userId) {
    return repo_.findByUserId(userId);
}

/// @brief Delegate to repository to fetch a single program.
Program ProgramService::getProgramById(int programId) {
    return repo_.findById(programId);
}

/// @brief Rename a program, enforcing uniqueness within the user's programs.
bool ProgramService::renameProgram(int programId, const std::string& newName) {
    // Validate name
    std::string trimmedName = trim(newName);
    if (trimmedName.empty()) {
        throw std::invalid_argument("Program name cannot be empty");
    }

    // Get existing program to check user and existence
    Program existing = repo_.findById(programId);

    // Check for duplicate name (excluding current program)
    if (repo_.existsByNameExcluding(existing.userId, trimmedName, programId)) {
        throw std::runtime_error("Program name already exists");
    }

    return repo_.update(programId, trimmedName);
}

/// @brief Delete a program after verifying it exists.
bool ProgramService::deleteProgram(int programId) {
    // Verify program exists first
    repo_.findById(programId); // throws if not found
    
    return repo_.deleteById(programId);
}
