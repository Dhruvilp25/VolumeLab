#include "exercise_repository.hpp"
#include <stdexcept>

int ExerciseRepository::insert(int userId, const std::string& name) {
    const std::string sql = "INSERT INTO exercises (user_id, name) VALUES (?, ?);";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, name);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            db_.finalize(stmt);
            throw std::runtime_error("Failed to insert exercise");
        }

        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        int exerciseId = 0;
        if (sqlite3_step(idStmt) == SQLITE_ROW) {
            exerciseId = Database::colInt(idStmt, 0);
        }
        db_.finalize(idStmt);
        db_.finalize(stmt);
        return exerciseId;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

Exercise ExerciseRepository::findById(int exerciseId) {
    const std::string sql = "SELECT exercise_id, user_id, name, is_archived, created_at FROM exercises WHERE exercise_id = ?;";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, exerciseId);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            db_.finalize(stmt);
            throw std::runtime_error("Exercise not found");
        }
        Exercise e;
        e.exerciseId = Database::colInt(stmt, 0);
        e.userId = Database::colInt(stmt, 1);
        e.name = Database::colText(stmt, 2);
        e.isArchived = Database::colInt(stmt, 3);
        e.createdAt = Database::colText(stmt, 4);
        db_.finalize(stmt);
        return e;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

Exercise ExerciseRepository::findByUserAndName(int userId, const std::string& name) {
    const std::string sql = "SELECT exercise_id, user_id, name, is_archived, created_at FROM exercises WHERE user_id = ? AND name = ?;";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, name);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            db_.finalize(stmt);
            throw std::runtime_error("Exercise not found");
        }
        Exercise e;
        e.exerciseId = Database::colInt(stmt, 0);
        e.userId = Database::colInt(stmt, 1);
        e.name = Database::colText(stmt, 2);
        e.isArchived = Database::colInt(stmt, 3);
        e.createdAt = Database::colText(stmt, 4);
        db_.finalize(stmt);
        return e;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

std::vector<Exercise> ExerciseRepository::findByUserId(int userId) {
    const std::string sql = "SELECT exercise_id, user_id, name, is_archived, created_at FROM exercises WHERE user_id = ? ORDER BY name;";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        std::vector<Exercise> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) throw std::runtime_error("findByUserId failed");
            Exercise e;
            e.exerciseId = Database::colInt(stmt, 0);
            e.userId = Database::colInt(stmt, 1);
            e.name = Database::colText(stmt, 2);
            e.isArchived = Database::colInt(stmt, 3);
            e.createdAt = Database::colText(stmt, 4);
            out.push_back(e);
        }
        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

bool ExerciseRepository::existsByUserAndName(int userId, const std::string& name) {
    const std::string sql = "SELECT COUNT(*) FROM exercises WHERE user_id = ? AND name = ?;";
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

int ExerciseRepository::findOrCreate(int userId, const std::string& name) {
    if (existsByUserAndName(userId, name)) {
        return findByUserAndName(userId, name).exerciseId;
    }
    return insert(userId, name);
}
