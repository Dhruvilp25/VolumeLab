#include "workout_session.hpp"
#include <stdexcept>

/**
 * @brief Starts a workout session.
 * @details Initializes the session with the given user ID.
 * @param userId The ID of the user starting the session.
 */
void WorkoutSession::startSession(int userId) {
    this->userId = userId;
}

/**
 * @brief Adds an exercise to the workout session.
 * @param exerciseId The ID of the exercise.
 * @param name Snapshot of the exercise name.
 */
void WorkoutSession::addExercise(int exerciseId, const std::string& name) {
    ExerciseEntry entry;
    entry.exerciseId = exerciseId;
    entry.exerciseNameSnapshot = name;
    exercises.push_back(entry);
}

/**
 * @brief Adds a set to an existing exercise in the session.
 * @details Searches for the exercise by ID and appends a new set.
 * @param exerciseId The ID of the exercise.
 * @param reps Number of repetitions performed.
 * @param weight Weight used for the set.
 * @throws std::runtime_error If the exercise is not found in the session.
 */
void WorkoutSession::addSet(int exerciseId, int reps, double weight) {
    for (auto& exercise : exercises) {
        if (exercise.exerciseId == exerciseId) {
            SetData set;
            set.reps = reps;
            set.weight = weight;
            exercise.sets.push_back(set);
            return;
        }
    }

    throw std::runtime_error("addSet: exercise not found in session");
}

/**
 * @brief Adds a set to an existing exercise in the session.
 * @details Searches for the exercise by ID and appends a new set.
 * @param exerciseId The ID of the exercise.
 * @param reps Number of repetitions performed.
 * @param weight Weight used for the set.
 * @throws std::runtime_error If the exercise is not found in the session.
 */
bool WorkoutSession::validate() {
    if (exercises.empty()) return false;

    for (const auto& exercise : exercises) {
        if (exercise.sets.empty()) return false;
    }

    return true;
}

/**
 * @brief Calculates the total workout volume.
 * @details Total volume is computed as the sum of (reps × weight)
 * across all exercises and sets in the session.
 * @return The total workout volume.
 */
double WorkoutSession::calculateTotalVolume() {
    double total = 0.0;

    for (const auto& exercise : exercises) {
        for (const auto& set : exercise.sets) {
            total += static_cast<double>(set.reps) * set.weight;
        }
    }

    return total;
}