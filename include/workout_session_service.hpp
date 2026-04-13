#pragma once
#include <string>
#include <vector>
#include "workout_session.hpp"
#include "workout_session_repository.hpp"

/**
 * @brief Provides application logic for workout session management.
 * @details This service coordinates workout creation, exercise logging,
 * validation, persistence, and retrieval of workout history.
 * It acts as the middle layer between the application and repository.
 * @author Fin Faniyi
 */
class WorkoutSessionService {
public:
    /**
     * @brief Constructs a workout session service.
     * @param repo Reference to the workout session repository.
     */
    explicit WorkoutSessionService(WorkoutSessionRepository& repo)
        : repo_(repo) {}

    /**
     * @brief Starts a new workout session for a user.
     * @details Currently acts as a placeholder; session initialization
     * is handled externally.
     * @param userId The ID of the user starting the workout.
     */
    void startWorkoutSession(int userId);

    /**
     * @brief Adds an exercise to the given workout session.
     * @param session The workout session being modified.
     * @param exerciseId The ID of the exercise.
     * @param exerciseName Snapshot of the exercise name.
     */
    void addExerciseToWorkout(
        WorkoutSession& session,
        int exerciseId,
        const std::string& exerciseName
    );

    /**
     * @brief Logs a set for an exercise in a workout session.
     * @param session The workout session being modified.
     * @param exerciseId The ID of the exercise.
     * @param reps Number of repetitions performed.
     * @param weight Weight used for the set.
     * @throws std::runtime_error If the exercise is not found in the session.
     */
    void logSet(
        WorkoutSession& session,
        int exerciseId,
        int reps,
        double weight
    );

    /**
     * @brief Validates a workout session before saving.
     * @details Ensures that the session contains at least one exercise
     * and that each exercise has at least one set.
     * @param session The workout session to validate.
     * @return True if the session is valid, false otherwise.
     */
    bool validateWorkout(const WorkoutSession& session);

    /**
     * @brief Ends and saves a workout session.
     * @details Validates the session, assigns a default source label if needed,
     * and persists the session, its exercises, and sets to the database.
     * @param session The workout session to finalize and save.
     * @throws std::runtime_error If the session is invalid.
     */
    void endWorkoutSession(WorkoutSession& session);

    /**
     * @brief Retrieves workout history for a user.
     * @param userId The ID of the user.
     * @return A vector of workout sessions belonging to the user.
     */
    std::vector<WorkoutSession> getWorkoutHistory(int userId);

    /**
     * @brief Loads full workout session details for display.
     * @param session Session with @c sessionId set.
     */
    void loadWorkoutSessionDetails(WorkoutSession& session);

private:
    WorkoutSessionRepository& repo_; /**< Repository used for database operations. */
};