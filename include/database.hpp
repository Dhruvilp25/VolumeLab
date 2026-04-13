#pragma once
#include <string>
#include <stdexcept>
#include "sqlite3.h"

/**
 * @brief RAII wrapper around an SQLite3 database connection.
 *
 * Manages the lifecycle of a single SQLite database file, providing methods
 * to open/close the connection, execute raw SQL, prepare and finalize
 * statements, run transactions, and safely bind/read typed values.
 * The class is non-copyable to prevent multiple owners of the same connection.
 *
 * @author Adrian Caricari
 */
class Database {
public:
    /**
     * @brief Constructs a Database object for the given file path.
     *
     * Does not open the connection; call open() to establish it.
     *
     * @param dbPath Filesystem path to the SQLite database file.
     */
    explicit Database(const std::string& dbPath);

    /**
     * @brief Destructor that closes the connection if still open.
     */
    ~Database();

    /// @brief Deleted copy constructor (database connections are non-copyable).
    Database(const Database&) = delete;
    /// @brief Deleted copy assignment operator (database connections are non-copyable).
    Database& operator=(const Database&) = delete;

    /**
     * @brief Opens the SQLite database connection and configures pragmas.
     *
     * Enables foreign keys, sets WAL journal mode, and sets synchronous to NORMAL.
     *
     * @throws std::runtime_error If the connection cannot be opened.
     */
    void open();

    /**
     * @brief Closes the SQLite database connection if currently open.
     */
    void close();

    /**
     * @brief Checks whether the database connection is currently open.
     * @return True if the connection is open and valid, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Begins a new SQLite transaction.
     */
    void beginTransaction();

    /**
     * @brief Commits the current SQLite transaction.
     */
    void commit();

    /**
     * @brief Rolls back the current SQLite transaction.
     */
    void rollback();

    /**
     * @brief Compiles an SQL string into a prepared statement.
     *
     * The caller is responsible for finalizing the returned statement
     * via finalize() when done.
     *
     * @param sql The SQL query string to compile.
     * @return Pointer to the compiled sqlite3_stmt.
     * @throws std::runtime_error If the database is not connected or preparation fails.
     */
    sqlite3_stmt* prepare(const std::string& sql);

    /**
     * @brief Finalizes (frees) a previously prepared statement.
     * @param stmt Pointer to the statement to finalize. Safe to pass nullptr.
     */
    void finalize(sqlite3_stmt* stmt);

    /**
     * @brief Executes a raw SQL string that does not return rows.
     *
     * Suitable for DDL, DML, and pragma statements.
     *
     * @param sql The SQL string to execute.
     * @throws std::runtime_error If the database is not connected or execution fails.
     */
    void exec(const std::string& sql);

    /**
     * @brief Returns the most recent error message stored by the database.
     * @return Const reference to the last error string.
     */
    const std::string& lastError() const { return lastError_; }

    // ── Bind helpers ───────────────────────────────────────────────

    /**
     * @brief Binds an integer value to a prepared statement parameter.
     * @param stmt  The prepared statement.
     * @param index 1-based parameter index.
     * @param value The integer value to bind.
     * @throws std::runtime_error If binding fails.
     */
    static void bindInt(sqlite3_stmt* stmt, int index, int value);

    /**
     * @brief Binds a 64-bit integer value to a prepared statement parameter.
     * @param stmt  The prepared statement.
     * @param index 1-based parameter index.
     * @param value The 64-bit integer value to bind.
     * @throws std::runtime_error If binding fails.
     */
    static void bindInt64(sqlite3_stmt* stmt, int index, long long value);

    /**
     * @brief Binds a double value to a prepared statement parameter.
     * @param stmt  The prepared statement.
     * @param index 1-based parameter index.
     * @param value The double value to bind.
     * @throws std::runtime_error If binding fails.
     */
    static void bindDouble(sqlite3_stmt* stmt, int index, double value);

    /**
     * @brief Binds a text string to a prepared statement parameter.
     * @param stmt  The prepared statement.
     * @param index 1-based parameter index.
     * @param value The string value to bind (copied via SQLITE_TRANSIENT).
     * @throws std::runtime_error If binding fails.
     */
    static void bindText(sqlite3_stmt* stmt, int index, const std::string& value);

    /**
     * @brief Binds a NULL value to a prepared statement parameter.
     * @param stmt  The prepared statement.
     * @param index 1-based parameter index.
     * @throws std::runtime_error If binding fails.
     */
    static void bindNull(sqlite3_stmt* stmt, int index);

    // ── Column helpers ─────────────────────────────────────────────

    /**
     * @brief Reads an integer value from a result row column.
     * @param stmt The stepped statement with a current result row.
     * @param col  0-based column index.
     * @return The integer value.
     */
    static int colInt(sqlite3_stmt* stmt, int col);

    /**
     * @brief Reads a 64-bit integer value from a result row column.
     * @param stmt The stepped statement with a current result row.
     * @param col  0-based column index.
     * @return The 64-bit integer value.
     */
    static long long colInt64(sqlite3_stmt* stmt, int col);

    /**
     * @brief Reads a double value from a result row column.
     * @param stmt The stepped statement with a current result row.
     * @param col  0-based column index.
     * @return The double value.
     */
    static double colDouble(sqlite3_stmt* stmt, int col);

    /**
     * @brief Reads a text string from a result row column.
     * @param stmt The stepped statement with a current result row.
     * @param col  0-based column index.
     * @return The text value, or an empty string if the column is NULL.
     */
    static std::string colText(sqlite3_stmt* stmt, int col);

private:
    /**
     * @brief Captures the current SQLite error message and throws a runtime_error.
     * @param context Description of the operation that failed.
     * @throws std::runtime_error Always thrown with context and SQLite error details.
     */
    void throwSqliteError(const std::string& context);

    std::string dbPath_;             ///< Filesystem path to the SQLite database file.
    sqlite3* conn_ = nullptr;        ///< Raw SQLite connection handle.
    bool isOpen_ = false;            ///< Whether the connection is currently open.
    std::string lastError_;          ///< Most recent error message.
};
