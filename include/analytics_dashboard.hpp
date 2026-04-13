#pragma once
#include <string>
#include <vector>
#include "weekly_trends.hpp"

/**
 * @brief Enumeration of preset date range options for the analytics dashboard.
 *
 * Defines fixed time windows (1 week, 1 month, 3 months, 1 year) and a
 * custom option that allows the caller to specify arbitrary start/end dates.
 *
 * @author Adrian Caricari
 */
enum class RangePreset { W1, M1, M3, Y1, Custom };

/**
 * @brief Data transfer object holding all analytics dashboard information for a date range.
 *
 * Aggregates workout summary statistics (counts, sets, reps, volume) together
 * with weekly trend rows so the UI can render both totals and time-series charts
 * from a single object.
 *
 * @author Adrian Caricari
 */
struct AnalyticsDashboard {
    bool hasWorkouts = false;            ///< Whether any workouts exist in the range.

    std::string startIsoInclusive;       ///< ISO date-time marking the start of the range (inclusive).
    std::string endIsoExclusive;         ///< ISO date-time marking the end of the range (exclusive).

    int workoutCount = 0;                ///< Total number of distinct workouts in the range.
    int totalSets = 0;                   ///< Total number of sets across all workouts.
    long long totalReps = 0;             ///< Total number of reps across all sets.
    double totalVolume = 0.0;            ///< Total volume (reps x weight) across all sets.

    std::vector<WeeklyTrendRow> weeklyTrends;  ///< Per-week breakdown of workout metrics.
};
