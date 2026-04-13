#include "user_repository.hpp"
#include "password_util.hpp"
#include <stdexcept>

int UserRepository::insert(const std::string& username, const std::string& passwordHash, const std::string& salt) {
    const std::string sql = R"SQL(
        INSERT INTO users (username, password_hash, salt)
        VALUES (?, ?, ?);
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindText(stmt, 1, username);
        Database::bindText(stmt, 2, passwordHash);
        Database::bindText(stmt, 3, salt);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            db_.finalize(stmt);
            throw std::runtime_error("Username already taken");
        }

        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        int userId = 0;
        if (sqlite3_step(idStmt) == SQLITE_ROW) {
            userId = Database::colInt(idStmt, 0);
        }
        db_.finalize(idStmt);
        db_.finalize(stmt);
        return userId;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

User UserRepository::findByUsername(const std::string& username) {
    const std::string sql = R"SQL(
        SELECT user_id, username, password_hash, salt, created_at
        FROM users WHERE username = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindText(stmt, 1, username);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            db_.finalize(stmt);
            throw std::runtime_error("User not found");
        }

        User u;
        u.userId = Database::colInt(stmt, 0);
        u.username = Database::colText(stmt, 1);
        u.passwordHash = Database::colText(stmt, 2);
        u.salt = Database::colText(stmt, 3);
        u.createdAt = Database::colText(stmt, 4);

        db_.finalize(stmt);
        return u;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

User UserRepository::findById(int userId) {
    const std::string sql = R"SQL(
        SELECT user_id, username, password_hash, salt, created_at
        FROM users WHERE user_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            db_.finalize(stmt);
            throw std::runtime_error("User not found");
        }

        User u;
        u.userId = Database::colInt(stmt, 0);
        u.username = Database::colText(stmt, 1);
        u.passwordHash = Database::colText(stmt, 2);
        u.salt = Database::colText(stmt, 3);
        u.createdAt = Database::colText(stmt, 4);

        db_.finalize(stmt);
        return u;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

bool UserRepository::existsByUsername(const std::string& username) {
    const std::string sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindText(stmt, 1, username);
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

bool UserRepository::deleteById(int userId) {
    const std::string sql = "DELETE FROM users WHERE user_id = ?;";
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, userId);
        int rc = sqlite3_step(stmt);
        db_.finalize(stmt);
        return rc == SQLITE_DONE;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}
