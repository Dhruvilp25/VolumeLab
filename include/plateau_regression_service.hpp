#pragma once
#include <string>
#include <vector>
#include "plateau_regression.hpp"
#include "plateau_regression_repository.hpp"

/**
 * @brief Service that detects plateau and regression trends across exercises.
 *
 * Consumes per-session best performance data from the PlateauRegressionRepository,
 * groups it by exercise, retains a sliding window of the most recent sessions,
 * and classifies each exercise's trend as OK, PLATEAU, REGRESSION, or
 * NOT_ENOUGH_DATA based on percentage-change thresholds.
 *
 * @author Adrian Caricari
 */
class PlateauRegressionService {
public:
    /**
     * @brief Constructs the service with its required repository.
     * @param repo Reference to the PlateauRegressionRepository for data access.
     */
    explicit PlateauRegressionService(PlateauRegressionRepository& repo)
        : repo_(repo) {}

    /**
     * @brief Detects exercise trends over a date range using recent session best values.
     *
     * For each exercise performed in the range, retains the most recent
     * minSessionsRequired sessions and classifies the trend by comparing the
     * first and last estimated 1RM values in that window.
     *
     * @param userId               The ID of the user.
     * @param startIsoInclusive    ISO datetime marking the start of the range (inclusive).
     * @param endIsoExclusive      ISO datetime marking the end of the range (exclusive).
     * @param minSessionsRequired  Minimum number of sessions needed to classify a trend (default 6).
     * @return Vector of ExerciseTrendFlag, one per exercise found in the range.
     */
    std::vector<ExerciseTrendFlag> detect(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        int minSessionsRequired = 6
    );

private:
    PlateauRegressionRepository& repo_;  ///< Repository for per-session best data.
};
