#include "volume_wrapped_repository.hpp"
#include <stdexcept>

// Author: Adrian Caricari

// Retrieves ranked exercise stats for a user over a date range
std::vector<VolumeWrappedExerciseStat> VolumeWrappedRepository::getExerciseLeaderboard(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    const std::string& orderBySql,
    int limit
) {
    // SQL query to aggregate exercise stats and order the results.
    const std::string sql = R"SQL(
        SELECT
            se.exercise_id,
            se.exercise_name_snapshot,
            COUNT(DISTINCT ws.session_id)              AS session_count,
            COUNT(ss.session_set_id)                   AS total_sets,
            COALESCE(SUM(ss.reps), 0)                  AS total_reps,
            COALESCE(SUM(ss.reps * ss.weight), 0)      AS total_volume
        FROM workout_sessions ws
        JOIN session_exercises se ON se.session_id = ws.session_id
        LEFT JOIN session_sets ss  ON ss.session_exercise_id = se.session_exercise_id
        WHERE ws.user_id = ?
          AND ws.performed_at >= ?
          AND ws.performed_at < ?
        GROUP BY se.exercise_id, se.exercise_name_snapshot
        ORDER BY )SQL" + orderBySql + R"SQL(
        LIMIT ?;
    )SQL";

    // Prepares the SQL statement
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds parameters to the query.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);
        Database::bindInt(stmt, 4, limit);

        // Iterates through each leaderboard row and stores it in the output vector.
        std::vector<VolumeWrappedExerciseStat> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("getExerciseLeaderboard: sqlite3_step failed");
            }

            VolumeWrappedExerciseStat row;
            row.exerciseId = Database::colInt(stmt, 0);
            row.exerciseName = Database::colText(stmt, 1);
            row.sessionCount = Database::colInt(stmt, 2);
            row.totalSets = Database::colInt(stmt, 3);
            row.totalReps = Database::colInt64(stmt, 4);
            row.totalVolume = Database::colDouble(stmt, 5);
            out.push_back(row);
        }

        // Finalizes the statement and return results
        db_.finalize(stmt);
        return out;
    } catch (...) {
        // Ensures the statement is finalized even if an error occurs.
        db_.finalize(stmt);
        throw;
    }
}

// Returns the total recorded workout duration in seconds for a user within a date range.
long long VolumeWrappedRepository::getTotalTrainingSeconds(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // SQL query to sum workout duration across the selected sessions.
    const std::string sql = R"SQL(
        SELECT COALESCE(SUM(duration_seconds), 0)
        FROM workout_sessions
        WHERE user_id = ?
          AND performed_at >= ?
          AND performed_at < ?;
    )SQL";

    // Prepares the SQL statement for execution.
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the user and date range parameters to the query
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Executes the query
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("getTotalTrainingSeconds: sqlite3_step failed");
        }

        // Reads the summed duration and returns it.
        long long total = Database::colInt64(stmt, 0);
        db_.finalize(stmt);
        return total;
    } catch (...) {
        // Ensures the statement is finalized if an error occurs.
        db_.finalize(stmt);
        throw;
    }
}

// Retrieves the current and best streak statistics for a user.
VolumeWrappedStreakStats VolumeWrappedRepository::getStreakStats(int userId) {
    // SQL query to fetch streak information for the user.
    const std::string sql = R"SQL(
        SELECT current_streak, best_streak, COALESCE(last_workout_date, '')
        FROM streaks
        WHERE user_id = ?;
    )SQL";

    // Prepares the SQL statement 
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the user id parameter to the query
        Database::bindInt(stmt, 1, userId);

        // Executes the query and reads the streak row if it exists.
        VolumeWrappedStreakStats stats;
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            stats.currentStreak = Database::colInt(stmt, 0);
            stats.bestStreak = Database::colInt(stmt, 1);
            stats.lastWorkoutDate = Database::colText(stmt, 2);
        } else if (rc != SQLITE_DONE) {
            throw std::runtime_error("getStreakStats: sqlite3_step failed");
        }

        // Finalizes the statement and returns the streak stats object.
        db_.finalize(stmt);
        return stats;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

// Returns the top exercises ranked by total volume within a date range.
std::vector<VolumeWrappedExerciseStat> VolumeWrappedRepository::getTopExercisesByVolume(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    int limit
) {
    // Reuses the shared leaderboard query with volume based ordering.
    return getExerciseLeaderboard(
        userId,
        startIsoInclusive,
        endIsoExclusive,
        "total_volume DESC, total_sets DESC, total_reps DESC, se.exercise_name_snapshot ASC",
        limit
    );
}

// Returns the top exercises ranked by total sets within a date range.
std::vector<VolumeWrappedExerciseStat> VolumeWrappedRepository::getTopExercisesBySets(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    int limit
) {
    // Reuses the shared leaderboard query with set count ordering.
    return getExerciseLeaderboard(
        userId,
        startIsoInclusive,
        endIsoExclusive,
        "total_sets DESC, total_volume DESC, total_reps DESC, se.exercise_name_snapshot ASC",
        limit
    );
}

// Retrieves the single most frequently repeated exercise within a date range.
bool VolumeWrappedRepository::getMostRepeatedExercise(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    VolumeWrappedExerciseStat& out
) {
    // Requests the top exercise ranked by number of sessions performed.
    const auto rows = getExerciseLeaderboard(
        userId,
        startIsoInclusive,
        endIsoExclusive,
        "session_count DESC, total_sets DESC, total_volume DESC, se.exercise_name_snapshot ASC",
        1
    );

    // Returns false when no exercise data exists, otherwise outputs the top row.
    if (rows.empty()) return false;
    out = rows.front();
    return true;
}

// Retrieves the most frequently performed template day within a date range.
bool VolumeWrappedRepository::getMostPerformedTemplateDay(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    VolumeWrappedTemplateDayStat& out
) {
    // SQL query to count performed sessions grouped by template day and label.
    const std::string sql = R"SQL(
        SELECT
            COALESCE(ws.source_template_day_id, 0) AS template_day_id,
            ws.source_label,
            COUNT(*) AS session_count
        FROM workout_sessions ws
        WHERE ws.user_id = ?
          AND ws.performed_at >= ?
          AND ws.performed_at < ?
        GROUP BY COALESCE(ws.source_template_day_id, 0), ws.source_label
        ORDER BY session_count DESC, ws.source_label ASC
        LIMIT 1;
    )SQL";

    // Prepares the SQL statement for execution.
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the user and date range parameters to the query.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Executes the query and returns the top template day if found.
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            out.templateDayId = Database::colInt(stmt, 0);
            out.dayName = Database::colText(stmt, 1);
            out.sessionCount = Database::colInt(stmt, 2);
            db_.finalize(stmt);
            return true;
        }
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("getMostPerformedTemplateDay: sqlite3_step failed");
        }

        // Returns false when no matching template day data exists.
        db_.finalize(stmt);
        return false;
    } catch (...) {
        // Ensures the statement is finalized if an error occurs.
        db_.finalize(stmt);
        throw;
    }
}

// Retrieves the single workout session with the highest total volume within a date range.
bool VolumeWrappedRepository::getBiggestVolumeDay(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    VolumeWrappedVolumeDay& out
) {
    // SQL query to aggregate each workout session and rank them by volume
    const std::string sql = R"SQL(
        SELECT
            ws.session_id,
            ws.performed_at,
            ws.source_label,
            COUNT(ss.session_set_id)                   AS total_sets,
            COALESCE(SUM(ss.reps), 0)                  AS total_reps,
            COALESCE(SUM(ss.reps * ss.weight), 0)      AS total_volume
        FROM workout_sessions ws
        LEFT JOIN session_exercises se ON se.session_id = ws.session_id
        LEFT JOIN session_sets ss      ON ss.session_exercise_id = se.session_exercise_id
        WHERE ws.user_id = ?
          AND ws.performed_at >= ?
          AND ws.performed_at < ?
        GROUP BY ws.session_id, ws.performed_at, ws.source_label
        ORDER BY total_volume DESC, total_sets DESC, ws.performed_at ASC
        LIMIT 1;
    )SQL";

    // Prepare the SQL statement 
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds parameters to query.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Execute the query 
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            out.sessionId = Database::colInt(stmt, 0);
            out.performedAtIso = Database::colText(stmt, 1);
            out.sourceLabel = Database::colText(stmt, 2);
            out.totalSets = Database::colInt(stmt, 3);
            out.totalReps = Database::colInt64(stmt, 4);
            out.totalVolume = Database::colDouble(stmt, 5);
            db_.finalize(stmt);
            return true;
        }
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("getBiggestVolumeDay: sqlite3_step failed");
        }

        // Returns false when no matching workout data exists.
        db_.finalize(stmt);
        return false;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}