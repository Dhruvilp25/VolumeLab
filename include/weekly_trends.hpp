#pragma once
#include <string>

/**
 * @brief Data row representing aggregated workout metrics for a single week.
 *
 * Each row corresponds to one ISO week, identified by the Monday date,
 * and holds totals for workouts, sets, reps, and volume within that week.
 *
 * @author Adrian Caricari
 */
struct WeeklyTrendRow {
    std::string weekStart;   ///< YYYY-MM-DD date of the Monday starting that week.
    int workoutCount = 0;    ///< Number of distinct workout sessions in the week.
    int totalSets = 0;       ///< Total number of sets performed in the week.
    long long totalReps = 0; ///< Total number of reps performed in the week.
    double totalVolume = 0.0; ///< Total volume (sum of reps x weight) in the week.
};
