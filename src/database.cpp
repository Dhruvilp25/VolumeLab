#include "database.hpp"
#include <iostream>

/**
 * @brief Constructs a Database object for the given file path.
 *
 * Stores the path but does not open the connection. Call open() to establish it.
 *
 * @param dbPath Filesystem path to the SQLite database file.
 *
 * @author Adrian Caricari
 */
Database::Database(const std::string& dbPath)
    : dbPath_(dbPath) {}

/**
 * @brief Destructor that closes the connection if still open.
 */
Database::~Database() {
    close();
}

/**
 * @brief Opens the SQLite database connection and configures pragmas.
 *
 * Enables foreign keys, sets WAL journal mode, and sets synchronous to NORMAL.
 * If the connection is already open, this method returns immediately.
 *
 * @throws std::runtime_error If sqlite3_open fails.
 *
 * @author Adrian Caricari
 */
void Database::open() {
    if (isOpen_) {
        return;
    }

    int rc = sqlite3_open(dbPath_.c_str(), &conn_);
    if (rc != SQLITE_OK) {
        lastError_ = conn_ ? sqlite3_errmsg(conn_) : "sqlite3_open failed";

        if (conn_) {
            sqlite3_close(conn_);
            conn_ = nullptr;
        }

        isOpen_ = false;
        throw std::runtime_error("Database open failed: " + lastError_);
    }

    isOpen_ = true;

    exec("PRAGMA foreign_keys = ON;");
    exec("PRAGMA journal_mode = WAL;");
    exec("PRAGMA synchronous = NORMAL;");
}

/**
 * @brief Closes the SQLite database connection if currently open.
 *
 * Safe to call multiple times; subsequent calls after the first are no-ops.
 */
void Database::close() {
    if (!isOpen_) {
        return; // already closed
    }

    if (conn_) {
        sqlite3_close(conn_);
        conn_ = nullptr;
    }

    isOpen_ = false;
}

/**
 * @brief Checks whether the database connection is currently open.
 * @return True if the connection is open and the handle is valid, false otherwise.
 */
bool Database::isConnected() const {
    return isOpen_ && conn_ != nullptr;
}

/**
 * @brief Begins a new SQLite transaction.
 * @throws std::runtime_error If the database is not connected or execution fails.
 */
void Database::beginTransaction() {
    exec("BEGIN TRANSACTION;");
}

/**
 * @brief Commits the current SQLite transaction.
 * @throws std::runtime_error If the database is not connected or execution fails.
 */
void Database::commit() {
    exec("COMMIT;");
}

/**
 * @brief Rolls back the current SQLite transaction.
 * @throws std::runtime_error If the database is not connected or execution fails.
 */
void Database::rollback() {
    exec("ROLLBACK;");
}

/**
 * @brief Compiles an SQL string into a prepared sqlite3_stmt object.
 *
 * The caller is responsible for finalizing the returned statement via finalize().
 *
 * @param sql The SQL query string to compile.
 * @return Pointer to the compiled sqlite3_stmt.
 * @throws std::runtime_error If the database is not connected or preparation fails.
 */
sqlite3_stmt* Database::prepare(const std::string& sql) {
    if (!isConnected()) {
        throw std::runtime_error("prepare() failed: database is not connected");
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        lastError_ = sqlite3_errmsg(conn_);
        throw std::runtime_error("Prepare failed: " + lastError_ + " | SQL: " + sql);
    }

    return stmt;
}

/**
 * @brief Finalizes (frees) a previously prepared statement.
 * @param stmt Pointer to the statement to finalize. Safe to pass nullptr.
 */
void Database::finalize(sqlite3_stmt* stmt) {
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief Executes a raw SQL string that does not return rows.
 *
 * Suitable for DDL statements, DML operations, and pragma configuration.
 *
 * @param sql The SQL string to execute.
 * @throws std::runtime_error If the database is not connected or execution fails.
 */
void Database::exec(const std::string& sql) {
    if (!isConnected()) {
        throw std::runtime_error("exec() failed: database is not connected");
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(conn_, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        lastError_ = errMsg ? errMsg : "sqlite3_exec failed";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL exec failed: " + lastError_ + " | SQL: " + sql);
    }
}

/**
 * @brief Captures the current SQLite error and throws a runtime_error.
 * @param context Description of the operation that failed.
 * @throws std::runtime_error Always thrown with context and error details.
 */
void Database::throwSqliteError(const std::string& context) {
    lastError_ = conn_ ? sqlite3_errmsg(conn_) : "sqlite error";
    throw std::runtime_error(context + ": " + lastError_);
}

// ── Bind helpers ───────────────────────────────────────────────────

/**
 * @brief Binds an integer value to a prepared statement parameter.
 * @param stmt  The prepared statement.
 * @param index 1-based parameter index.
 * @param value The integer value to bind.
 * @throws std::runtime_error If binding fails.
 */
void Database::bindInt(sqlite3_stmt* stmt, int index, int value) {
    int rc = sqlite3_bind_int(stmt, index, value);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("bindInt failed");
    }
}

/**
 * @brief Binds a 64-bit integer value to a prepared statement parameter.
 * @param stmt  The prepared statement.
 * @param index 1-based parameter index.
 * @param value The 64-bit integer value to bind.
 * @throws std::runtime_error If binding fails.
 */
void Database::bindInt64(sqlite3_stmt* stmt, int index, long long value) {
    int rc = sqlite3_bind_int64(stmt, index, static_cast<sqlite3_int64>(value));
    if (rc != SQLITE_OK) {
        throw std::runtime_error("bindInt64 failed");
    }
}

/**
 * @brief Binds a double value to a prepared statement parameter.
 * @param stmt  The prepared statement.
 * @param index 1-based parameter index.
 * @param value The double value to bind.
 * @throws std::runtime_error If binding fails.
 */
void Database::bindDouble(sqlite3_stmt* stmt, int index, double value) {
    int rc = sqlite3_bind_double(stmt, index, value);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("bindDouble failed");
    }
}

/**
 * @brief Binds a text string to a prepared statement parameter.
 * @param stmt  The prepared statement.
 * @param index 1-based parameter index.
 * @param value The string value to bind (copied via SQLITE_TRANSIENT).
 * @throws std::runtime_error If binding fails.
 */
void Database::bindText(sqlite3_stmt* stmt, int index, const std::string& value) {
    int rc = sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("bindText failed");
    }
}

/**
 * @brief Binds a NULL value to a prepared statement parameter.
 * @param stmt  The prepared statement.
 * @param index 1-based parameter index.
 * @throws std::runtime_error If binding fails.
 */
void Database::bindNull(sqlite3_stmt* stmt, int index) {
    int rc = sqlite3_bind_null(stmt, index);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("bindNull failed");
    }
}

// ── Column helpers ─────────────────────────────────────────────────

/**
 * @brief Reads an integer value from a result row column.
 * @param stmt The stepped statement with a current result row.
 * @param col  0-based column index.
 * @return The integer value.
 */
int Database::colInt(sqlite3_stmt* stmt, int col) {
    return sqlite3_column_int(stmt, col);
}

/**
 * @brief Reads a 64-bit integer value from a result row column.
 * @param stmt The stepped statement with a current result row.
 * @param col  0-based column index.
 * @return The 64-bit integer value.
 */
long long Database::colInt64(sqlite3_stmt* stmt, int col) {
    return static_cast<long long>(sqlite3_column_int64(stmt, col));
}

/**
 * @brief Reads a double value from a result row column.
 * @param stmt The stepped statement with a current result row.
 * @param col  0-based column index.
 * @return The double value.
 */
double Database::colDouble(sqlite3_stmt* stmt, int col) {
    return sqlite3_column_double(stmt, col);
}

/**
 * @brief Reads a text string from a result row column.
 * @param stmt The stepped statement with a current result row.
 * @param col  0-based column index.
 * @return The text value, or an empty string if the column is NULL.
 */
std::string Database::colText(sqlite3_stmt* stmt, int col) {
    const unsigned char* txt = sqlite3_column_text(stmt, col);
    return txt ? reinterpret_cast<const char*>(txt) : "";
}
