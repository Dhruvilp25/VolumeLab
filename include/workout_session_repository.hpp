#pragma once
#include <string>
#include <vector>
#include "database.hpp"
#include "workout_session.hpp"


/**
 * @brief Handles database operations for workout sessions.
 * @details This repository is responsible for saving workout sessions,
 * inserting exercise entries and sets, and retrieving workout history
 * from the SQLite database.
 * @author Fin Faniyi
 */
class WorkoutSessionRepository {
public:

    /**
     * @brief Constructs a workout session repository.
     * @param db Reference to the database used for persistence.
     */
    explicit WorkoutSessionRepository(Database& db) : db_(db) {}

    /**
     * @brief Saves a completed workout session and returns its generated ID.
     * @param session The workout session to save.
     * @return The generated workout session ID.
     */
    int saveWorkoutSession(const WorkoutSession& session);

    /**
     * @brief Inserts an exercise entry for a workout session.
     * @param sessionId The ID of the workout session.
     * @param exerciseId The ID of the exercise being logged.
     * @param exerciseName Snapshot of the exercise name at time of logging.
     * @return The generated session exercise ID.
     */
    int insertExerciseEntry(
        int sessionId,
        int exerciseId,
        const std::string& exerciseName
    );

    /**
     * @brief Inserts a set for a logged session exercise.
     * @param sessionExerciseId The ID of the session-specific exercise entry.
     * @param setNumber The set number within that exercise entry.
     * @param reps Number of repetitions performed.
     * @param weight Weight used for the set.
     * @return void
     */
    void insertSetData(
        int sessionExerciseId,
        int setNumber,
        int reps,
        double weight
    );

    /**
     * @brief Retrieves workout history for a user.
     * @param userId The ID of the user.
     * @return A vector of workout sessions belonging to the user.
     */
    std::vector<WorkoutSession> getWorkoutHistory(int userId);

    /**
     * @brief Loads exercises and sets for a workout session.
     * @details Populates @c session.exercises with all logged exercises and sets.
     * @param session Session object with @c sessionId set.
     */
    void loadWorkoutSessionDetails(WorkoutSession& session);

private:
    Database& db_;
};