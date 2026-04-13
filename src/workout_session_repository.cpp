#include "workout_session_repository.hpp"
#include <stdexcept>
#include <unordered_map>

// Author: Fin Faniyi

/**
 * @brief Saves a completed workout session to the database.
 * @details Inserts a new record into the workout_sessions table and
 * retrieves the generated session ID using SQLite.
 * @param session The workout session to be saved.
 * @return The generated session ID.
 */
int WorkoutSessionRepository::saveWorkoutSession(const WorkoutSession& session) {

    const std::string sql = R"SQL(
        INSERT INTO workout_sessions (user_id, performed_at, duration_seconds, source_label)
        VALUES (?, ?, ?, ?);
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);

    try {
        Database::bindInt(stmt, 1, session.userId);
        Database::bindText(stmt, 2, session.performedAt);
        Database::bindInt(stmt, 3, session.durationSeconds);
        Database::bindText(stmt, 4, session.sourceLabel);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("saveWorkoutSession: sqlite3_step failed");
        }

        db_.finalize(stmt);
        stmt = nullptr;

        // Retrieve the generated workout session ID
        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        try {
            rc = sqlite3_step(idStmt);
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("saveWorkoutSession: failed to retrieve inserted session ID");
            }

            int sessionId = Database::colInt(idStmt, 0);
            db_.finalize(idStmt);
            return sessionId;
        } catch (...) {
            db_.finalize(idStmt);
            throw;
        }
    }
    catch (...) {
        db_.finalize(stmt);
        throw;
    }
}


/**
 * @brief Inserts an exercise entry for a workout session.
 * @details Adds a record to the session_exercises table and retrieves
 * the generated session_exercise ID.
 * @param sessionId The ID of the workout session.
 * @param exerciseId The ID of the exercise being logged.
 * @param exerciseName Snapshot of the exercise name.
 * @return The generated session exercise ID.
 */
int WorkoutSessionRepository::insertExerciseEntry(
    int sessionId,
    int exerciseId,
    const std::string& exerciseName
) {

    const std::string sql = R"SQL(
        INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot)
        VALUES (?, ?, ?);
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);

    try {
        Database::bindInt(stmt, 1, sessionId);
        Database::bindInt(stmt, 2, exerciseId);
        Database::bindText(stmt, 3, exerciseName);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("insertExerciseEntry: sqlite3_step failed");
        }

        db_.finalize(stmt);
        stmt = nullptr;

        // Retrieve the generated session_exercise ID
        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        try {
            rc = sqlite3_step(idStmt);
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("insertExerciseEntry: failed to retrieve inserted session exercise ID");
            }

            int sessionExerciseId = Database::colInt(idStmt, 0);
            db_.finalize(idStmt);
            return sessionExerciseId;
        } catch (...) {
            db_.finalize(idStmt);
            throw;
        }
    }
    catch (...) {
        db_.finalize(stmt);
        throw;
    }
}


/**
 * @brief Inserts a set for a session exercise.
 * @details Adds a record to the session_sets table, including set number,
 * repetitions, and weight.
 * @param sessionExerciseId The ID of the session-specific exercise entry.
 * @param setNumber The set number within the exercise.
 * @param reps Number of repetitions performed.
 * @param weight Weight used for the set.
 */
void WorkoutSessionRepository::insertSetData(
    int sessionExerciseId,
    int setNumber,
    int reps,
    double weight
) {

    const std::string sql = R"SQL(
        INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
        VALUES (?, ?, ?, ?);
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);

    try {
        Database::bindInt(stmt, 1, sessionExerciseId);
        Database::bindInt(stmt, 2, setNumber);
        Database::bindInt(stmt, 3, reps);
        Database::bindDouble(stmt, 4, weight);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("insertSetData: sqlite3_step failed");
        }

        db_.finalize(stmt);
    }
    catch (...) {
        db_.finalize(stmt);
        throw;
    }
}


/**
 * @brief Retrieves workout history for a user.
 * @details Fetches all workout sessions belonging to a user,
 * ordered from most recent to oldest.
 * @param userId The ID of the user.
 * @return A vector of workout sessions.
 */
std::vector<WorkoutSession> WorkoutSessionRepository::getWorkoutHistory(int userId) {

    const std::string sql = R"SQL(
        SELECT session_id, user_id, performed_at, duration_seconds, source_label
        FROM workout_sessions
        WHERE user_id = ?
        ORDER BY performed_at DESC;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);

    try {
        Database::bindInt(stmt, 1, userId);

        std::vector<WorkoutSession> out;

        while (true) {
            int rc = sqlite3_step(stmt);

            if (rc == SQLITE_DONE) break;

            if (rc != SQLITE_ROW) {
                throw std::runtime_error("getWorkoutHistory: sqlite3_step failed");
            }

            WorkoutSession session;
            session.sessionId = Database::colInt(stmt, 0);
            session.userId = Database::colInt(stmt, 1);
            session.performedAt = Database::colText(stmt, 2);
            session.durationSeconds = Database::colInt(stmt, 3);
            session.sourceLabel = Database::colText(stmt, 4);

            out.push_back(session);
        }

        db_.finalize(stmt);
        return out;
    }
    catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/**
 * @brief Loads full workout session details (exercises + sets).
 * @details Queries session_exercises and session_sets to populate the in-memory model.
 * @param session Session with a valid sessionId.
 */
void WorkoutSessionRepository::loadWorkoutSessionDetails(WorkoutSession& session) {
    session.exercises.clear();

    // Load exercises first (preserve insertion order by session_exercise_id)
    const std::string exSql = R"SQL(
        SELECT session_exercise_id, exercise_id, exercise_name_snapshot
        FROM session_exercises
        WHERE session_id = ?
        ORDER BY sort_order ASC, session_exercise_id ASC;
    )SQL";

    sqlite3_stmt* exStmt = db_.prepare(exSql);
    try {
        Database::bindInt(exStmt, 1, session.sessionId);

        std::vector<int> sessionExerciseIds;
        std::unordered_map<int, size_t> seidToIdx;

        while (true) {
            int rc = sqlite3_step(exStmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("loadWorkoutSessionDetails: loading exercises failed");
            }

            int sessionExerciseId = Database::colInt(exStmt, 0);
            ExerciseEntry entry;
            entry.exerciseId = Database::colInt(exStmt, 1);
            entry.exerciseNameSnapshot = Database::colText(exStmt, 2);

            seidToIdx[sessionExerciseId] = session.exercises.size();
            session.exercises.push_back(entry);
            sessionExerciseIds.push_back(sessionExerciseId);
        }

        db_.finalize(exStmt);
        exStmt = nullptr;

        // Load sets for all exercises in this session
        const std::string setSql = R"SQL(
            SELECT session_exercise_id, set_number, reps, weight
            FROM session_sets
            WHERE session_exercise_id IN (
                SELECT session_exercise_id FROM session_exercises WHERE session_id = ?
            )
            ORDER BY session_exercise_id ASC, set_number ASC;
        )SQL";

        sqlite3_stmt* setStmt = db_.prepare(setSql);
        try {
            Database::bindInt(setStmt, 1, session.sessionId);

            while (true) {
                int rc = sqlite3_step(setStmt);
                if (rc == SQLITE_DONE) break;
                if (rc != SQLITE_ROW) {
                    throw std::runtime_error("loadWorkoutSessionDetails: loading sets failed");
                }

                int sessionExerciseId = Database::colInt(setStmt, 0);
                int reps = Database::colInt(setStmt, 2);
                double weight = Database::colDouble(setStmt, 3);

                auto it = seidToIdx.find(sessionExerciseId);
                if (it == seidToIdx.end()) continue;

                SetData s;
                s.reps = reps;
                s.weight = weight;
                session.exercises[it->second].sets.push_back(s);
            }

            db_.finalize(setStmt);
        } catch (...) {
            db_.finalize(setStmt);
            throw;
        }
    } catch (...) {
        db_.finalize(exStmt);
        throw;
    }
}