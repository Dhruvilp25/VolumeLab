#include "analytics_dashboard_repository.hpp"
#include <stdexcept>

/**
 * @brief Retrieves aggregate workout totals for a user within a given date range.
 *
 * Executes a single SQL query joining workout_sessions, session_exercises, and
 * session_sets to compute the count of distinct workouts, total sets, total reps,
 * and total volume (reps x weight) for the specified user and date window.
 *
 * @param userId             The ID of the user whose totals are requested.
 * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
 * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
 * @return A RangeTotals struct populated with the aggregated values.
 * @throws std::runtime_error If the SQL query execution fails.
 *
 * @author Adrian Caricari
 */
RangeTotals AnalyticsDashboardRepository::getRangeTotals(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // SQL query to count workouts, sets, reps, and total volume in the selected range.
    const std::string sql = R"SQL(
        SELECT
            COUNT(DISTINCT ws.session_id) AS workout_count,
            COUNT(ss.session_set_id)      AS total_sets,
            COALESCE(SUM(ss.reps), 0)     AS total_reps,
            COALESCE(SUM(ss.reps * ss.weight), 0) AS total_volume
        FROM workout_sessions ws
        LEFT JOIN session_exercises se ON se.session_id = ws.session_id
        LEFT JOIN session_sets ss      ON ss.session_exercise_id = se.session_exercise_id
        WHERE ws.user_id = ?
          AND ws.performed_at >= ?
          AND ws.performed_at < ?;
    )SQL";

    // Prepares the SQL statement.
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the function inputs to the SQL query parameters.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Executes the query and checks for a valid SQLite result state
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            throw std::runtime_error("getRangeTotals: sqlite3_step failed");
        }

        // Reads the returned values into a RangeTotals object.
        RangeTotals t;
        if (rc == SQLITE_ROW) {
            t.workoutCount = Database::colInt(stmt, 0);
            t.totalSets    = Database::colInt(stmt, 1);
            t.totalReps    = Database::colInt64(stmt, 2);
            t.totalVolume  = Database::colDouble(stmt, 3);
        }

        // Finalizes the statement and returns the populated totals object.
        db_.finalize(stmt);
        return t;

    } catch (...) {
        // Ensures the statement is finalized even if an error occurs.
        db_.finalize(stmt);
        throw;
    }
}
