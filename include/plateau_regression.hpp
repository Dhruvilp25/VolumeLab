#pragma once
#include <string>

/**
 * @brief Status classification for an exercise's recent performance trend.
 *
 * Used by the plateau/regression detection system to flag whether an exercise
 * is progressing normally, has stalled, or is declining.
 *
 * @author Adrian Caricari
 */
enum class TrendFlagStatus {
    OK,               ///< Performance is progressing normally.
    PLATEAU,          ///< Performance has stalled (change within plateau threshold).
    REGRESSION,       ///< Performance is declining (negative change beyond threshold).
    NOT_ENOUGH_DATA   ///< Insufficient sessions to make a determination.
};

/**
 * @brief Holds the trend analysis result for a single exercise.
 *
 * Contains the exercise identity, the metric being tracked, the analysis
 * window boundaries, session counts, first/last metric values, and the
 * computed trend status.
 *
 * @author Adrian Caricari
 */
struct ExerciseTrendFlag {
    int exerciseId = 0;              ///< Unique identifier of the exercise.
    std::string exerciseName;        ///< Human-readable name of the exercise.

    TrendFlagStatus status = TrendFlagStatus::NOT_ENOUGH_DATA;  ///< Detected trend status.

    std::string metricName;          ///< Name of the tracked metric (e.g. "estimated_1rm_epley").
    std::string windowStartIso;      ///< ISO datetime of the first session in the analysis window.
    std::string windowEndIso;        ///< ISO datetime of the last session in the analysis window.

    int sessionsUsed = 0;            ///< Number of sessions included in the analysis window.
    int minSessionsRequired = 0;     ///< Minimum sessions needed for a valid determination.

    double firstValue = 0.0;         ///< Metric value from the first session in the window.
    double lastValue = 0.0;          ///< Metric value from the last session in the window.
    double delta = 0.0;              ///< Change in metric value (lastValue - firstValue).
};
