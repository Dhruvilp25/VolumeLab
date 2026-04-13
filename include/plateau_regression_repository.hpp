#pragma once
#include <string>
#include <vector>
#include "database.hpp"

/**
 * @brief Data row holding the best estimated 1RM for one exercise in one session.
 *
 * Represents a single row from the query that finds the highest Epley-estimated
 * one-rep max for each exercise within each workout session.
 *
 * @author Adrian Caricari
 */
struct ExerciseSessionBest {
    int exerciseId = 0;              ///< Unique identifier of the exercise.
    std::string exerciseName;        ///< Snapshot of the exercise name at session time.
    std::string performedAtIso;      ///< ISO datetime of the workout session.
    double bestMetric = 0.0;         ///< Best estimated 1RM (Epley) in that session for the exercise.
};

/**
 * @brief Repository that retrieves per-session best performance data for trend analysis.
 *
 * Queries the database to compute the maximum estimated 1RM (using the Epley formula)
 * for each exercise in each workout session within a given date range. The results
 * are consumed by PlateauRegressionService to detect plateaus and regressions.
 *
 * @author Adrian Caricari
 */
class PlateauRegressionRepository {
public:
    /**
     * @brief Constructs the repository with a reference to an open database.
     * @param db Reference to the Database object used for queries.
     */
    explicit PlateauRegressionRepository(Database& db) : db_(db) {}

    /**
     * @brief Retrieves the best estimated 1RM per exercise per session within a date range.
     *
     * Uses the Epley formula (weight x (1 + reps/30)) to estimate 1RM for each set,
     * then takes the maximum per exercise per session. Results are ordered by
     * exercise ID ascending, then by performed_at ascending.
     *
     * @param userId             The ID of the user.
     * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
     * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
     * @return Vector of ExerciseSessionBest rows sorted by exercise then date.
     */
    std::vector<ExerciseSessionBest> getExerciseSessionBests(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

private:
    Database& db_;  ///< Reference to the database connection.
};
