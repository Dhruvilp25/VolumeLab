#include "plateau_regression_repository.hpp"
#include <stdexcept>

/**
 * @brief Retrieves the best estimated 1RM for each exercise in each session within a date range.
 *
 * Joins workout_sessions, session_exercises, and session_sets, computes the Epley
 * estimated 1RM (weight x (1 + reps/30)) for every set, and returns the maximum
 * per exercise per session. Results are ordered by exercise ID ascending then
 * performed_at ascending so the caller can process them chronologically per exercise.
 *
 * @param userId             The ID of the user.
 * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
 * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
 * @return Vector of ExerciseSessionBest rows sorted by exercise then date.
 * @throws std::runtime_error If the SQL query execution fails.
 *
 * @author Adrian Caricari
 */
std::vector<ExerciseSessionBest> PlateauRegressionRepository::getExerciseSessionBests(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // SQL query to compute the best e1RM per exercise per session.
    const std::string sql = R"SQL(
        SELECT
            se.exercise_id,
            se.exercise_name_snapshot,
            ws.performed_at,
            MAX(ss.weight * (1.0 + (ss.reps / 30.0))) AS best_e1rm
        FROM workout_sessions ws
        JOIN session_exercises se ON se.session_id = ws.session_id
        JOIN session_sets ss      ON ss.session_exercise_id = se.session_exercise_id
        WHERE ws.user_id = ?
          AND ws.performed_at >= ?
          AND ws.performed_at < ?
          AND se.exercise_id IS NOT NULL
        GROUP BY se.exercise_id, ws.session_id
        ORDER BY se.exercise_id ASC, ws.performed_at ASC;
    )SQL";

    // Prepares the SQL statement for execution.
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the user and date range parameters to the query.
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);

        // Iterates through each result row and stores it
        std::vector<ExerciseSessionBest> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) throw std::runtime_error("getExerciseSessionBests: sqlite3_step failed");

            ExerciseSessionBest row;
            row.exerciseId      = Database::colInt(stmt, 0);
            row.exerciseName    = Database::colText(stmt, 1);
            row.performedAtIso  = Database::colText(stmt, 2);
            row.bestMetric      = Database::colDouble(stmt, 3);
            out.push_back(row);
        }

        // Finalizes the statement and returns the collected results.
        db_.finalize(stmt);
        return out;
    } catch (...) {
        // Ensures the statement is finalized if an error occurs.
        db_.finalize(stmt);
        throw;
    }
}
