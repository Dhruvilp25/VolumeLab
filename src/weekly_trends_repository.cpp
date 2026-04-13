#include "weekly_trends_repository.hpp"
#include <stdexcept>

/**
 * @brief Retrieves weekly workout summary rows for a user within a given date range.
 *
 * Groups workout sessions by the Monday of each ISO week using SQLite date
 * functions, then computes per-week totals for workouts, sets, reps, and volume.
 * Results are returned in ascending week order.
 *
 * @param userId             The ID of the user.
 * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
 * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
 * @return Vector of WeeklyTrendRow sorted by weekStart ascending.
 * @throws std::runtime_error If the SQL query execution fails.
 *
 * @author Adrian Caricari
 */
std::vector<WeeklyTrendRow> WeeklyTrendsRepository::getWeeklyTrends(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // SQL query to group sessions by the Monday of each week and calculate weekly totals.
    const std::string sql = R"SQL(
        WITH sessions_in_range AS (
            SELECT
                ws.session_id,
                date(
                    ws.performed_at,
                    printf('-%d days', ((CAST(strftime('%w', ws.performed_at) AS INTEGER) + 6) % 7))
                ) AS week_start
            FROM workout_sessions ws
            WHERE ws.user_id = ?
              AND ws.performed_at >= ?
              AND ws.performed_at < ?
        )
        SELECT
            s.week_start,
            COUNT(DISTINCT s.session_id) AS workout_count,
            COUNT(ss.session_set_id)     AS total_sets,
            COALESCE(SUM(ss.reps), 0)    AS total_reps,
            COALESCE(SUM(ss.reps * ss.weight), 0) AS total_volume
        FROM sessions_in_range s
        LEFT JOIN session_exercises se ON se.session_id = s.session_id
        LEFT JOIN session_sets ss      ON ss.session_exercise_id = se.session_exercise_id
        GROUP BY s.week_start
        ORDER BY s.week_start ASC;
    )SQL";

    // Prepares the SQL statement
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the user and date range values to the SQL query.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Steps through each weekly result row and stores it in the output vector.
        std::vector<WeeklyTrendRow> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("getWeeklyTrends: sqlite3_step failed");
            }

            WeeklyTrendRow row;
            row.weekStart    = Database::colText(stmt, 0);
            row.workoutCount = Database::colInt(stmt, 1);
            row.totalSets    = Database::colInt(stmt, 2);
            row.totalReps    = Database::colInt64(stmt, 3);
            row.totalVolume  = Database::colDouble(stmt, 4);
            out.push_back(row);
        }

        // Finalizes the statement
        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}
