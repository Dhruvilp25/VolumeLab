#pragma once
#include <vector>
#include <string>

/**
 * @brief Represents a single set performed during an exercise.
 * @details Stores the number of repetitions and the weight used for one set.
 * @author Fin Faniyi
 */
struct SetData {
    int reps;       /**< Number of repetitions performed. */
    double weight;  /**< Weight used for the set. */
};


/**
 * @brief Represents an exercise logged within a workout session.
 * @details Stores the exercise identifier, a snapshot of the exercise name,
 * and all sets performed for that exercise during the session.
 * @author Fin Faniyi
 */
struct ExerciseEntry {
    int exerciseId;                   /**< Unique ID of the exercise. */
    std::string exerciseNameSnapshot; /**< Name of the exercise at time of logging. */
    std::vector<SetData> sets;        /**< Sets performed for this exercise. */
};


/**
 * @brief Represents a workout session for a user.
 * @details Stores workout session metadata along with all exercises and sets
 * performed during the session.
 * @author Fin Faniyi
 */
class WorkoutSession {
public:
    int sessionId;                 /**< Unique ID of the workout session. */
    int userId;                    /**< ID of the user who owns the session. */
    std::string performedAt;       /**< Timestamp when the workout was performed. */
    int durationSeconds;           /**< Duration of the workout in seconds. */
    std::string sourceLabel;       /**< Label for session source (required by DB). */

    std::vector<ExerciseEntry> exercises; /**< Exercises performed in this session. */

    /**
     * @brief Starts a workout session.
     * @param userId The ID of the user.
     */
    void startSession(int userId);

    /**
     * @brief Adds an exercise to the session.
     * @param exerciseId The exercise ID.
     * @param name Snapshot of exercise name.
     */
    void addExercise(int exerciseId, const std::string& name);

    /**
     * @brief Adds a set to an exercise.
     * @param exerciseId The exercise ID.
     * @param reps Number of reps.
     * @param weight Weight used.
     */
    void addSet(int exerciseId, int reps, double weight);

    /**
     * @brief Validates the session.
     * @return True if valid, false otherwise.
     */
    bool validate();

    /**
     * @brief Calculates total volume.
     * @return Total volume (reps * weight).
     */
    double calculateTotalVolume();
};