#include "workout_session_service.hpp"
#include <stdexcept>

// Author: Fin Faniyi

/**
 * @brief Starts a workout session.
 * @details Currently acts as a placeholder. Session initialization
 * is handled externally by the caller.
 * @param userId The ID of the user starting the workout.
 */
void WorkoutSessionService::startWorkoutSession(int userId) {
    (void)userId;
    // Session setup is currently handled by the caller.
}

/**
 * @brief Adds an exercise to an in-memory workout session.
 * @param session The workout session being modified.
 * @param exerciseId The ID of the exercise.
 * @param exerciseName Snapshot of the exercise name.
 */
void WorkoutSessionService::addExerciseToWorkout(
    WorkoutSession& session,
    int exerciseId,
    const std::string& exerciseName
) {
    ExerciseEntry entry;
    entry.exerciseId = exerciseId;
    entry.exerciseNameSnapshot = exerciseName;

    session.exercises.push_back(entry);
}

/**
 * @brief Logs a set for an exercise in the session.
 * @details Searches for the matching exercise and appends a new set.
 * Throws an error if the exercise is not found.
 * @param session The workout session being modified.
 * @param exerciseId The ID of the exercise.
 * @param reps Number of repetitions performed.
 * @param weight Weight used for the set.
 * @throws std::runtime_error If the exercise does not exist in the session.
 */
void WorkoutSessionService::logSet(
    WorkoutSession& session,
    int exerciseId,
    int reps,
    double weight
) {
    for (auto& exercise : session.exercises) {
        if (exercise.exerciseId == exerciseId) {
            SetData set;
            set.reps = reps;
            set.weight = weight;

            exercise.sets.push_back(set);
            return;
        }
    }

    throw std::runtime_error("logSet: exercise not found in session");
}

/**
 * @brief Validates a workout session before saving.
 * @details Ensures that the session contains at least one exercise
 * and that each exercise has at least one set.
 * @param session The workout session to validate.
 * @return True if the session is valid, false otherwise.
 */
bool WorkoutSessionService::validateWorkout(const WorkoutSession& session) {
    if (session.exercises.empty()) return false;

    for (const auto& exercise : session.exercises) {
        if (exercise.sets.empty()) return false;
    }

    return true;
}

/**
 * @brief Ends and saves a workout session.
 * @details Validates the session, assigns a default source label if needed,
 * saves the session to the database, and inserts all exercises and sets.
 * @param session The workout session to finalize and persist.
 * @throws std::runtime_error If the session is invalid.
 */
void WorkoutSessionService::endWorkoutSession(WorkoutSession& session) {
    if (!validateWorkout(session)) {
        throw std::runtime_error("endWorkoutSession: invalid workout session");
    }

    if (session.sourceLabel.empty()) {
        session.sourceLabel = "manual";
    }

    // Save the session and store its generated ID
    int sessionId = repo_.saveWorkoutSession(session);
    session.sessionId = sessionId;

    // Save each exercise entry and all of its sets
    for (const auto& exercise : session.exercises) {
        int sessionExerciseId = repo_.insertExerciseEntry(
            sessionId,
            exercise.exerciseId,
            exercise.exerciseNameSnapshot
        );

        int setNumber = 1;
        for (const auto& set : exercise.sets) {
            repo_.insertSetData(
                sessionExerciseId,
                setNumber,
                set.reps,
                set.weight
            );
            setNumber++;
        }
    }
}

/**
 * @brief Retrieves workout history for a user.
 * @param userId The ID of the user.
 * @return A vector of workout sessions.
 */
std::vector<WorkoutSession> WorkoutSessionService::getWorkoutHistory(int userId) {
    return repo_.getWorkoutHistory(userId);
}

void WorkoutSessionService::loadWorkoutSessionDetails(WorkoutSession& session) {
    repo_.loadWorkoutSessionDetails(session);
}