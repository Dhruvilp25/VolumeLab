#include "plateau_regression_service.hpp"
#include <unordered_map>
#include <deque>
#include <cmath>

/**
 * @brief Classifies a recent performance window as OK, plateau, or regression.
 *
 * Compares the first and last estimated 1RM values in the window and applies
 * percentage-change thresholds to determine the trend status.
 *
 * @param window                Deque of session-best rows for a single exercise (chronological).
 * @param plateauEpsilonPct     Maximum absolute percentage change to classify as PLATEAU.
 * @param regressionEpsilonPct  Minimum negative percentage change to classify as REGRESSION.
 * @return TrendFlagStatus indicating OK, PLATEAU, or REGRESSION.
 *
 * @author Adrian Caricari
 */
static TrendFlagStatus classifyWindow(
    const std::deque<ExerciseSessionBest>& window,
    double plateauEpsilonPct,
    double regressionEpsilonPct
) {
    // Returns OK when there are not enough data
    if (window.size() < 2) return TrendFlagStatus::OK;

    // Reads the first and last values in the window for trend comparison.
    const double first = window.front().bestMetric;
    const double last  = window.back().bestMetric;

    const double denom = (std::abs(first) < 1e-9) ? 1.0 : std::abs(first);
    const double pctChange = (last - first) / denom;

    // Applies threshold rules to classify the trend.
    if (pctChange <= -regressionEpsilonPct) return TrendFlagStatus::REGRESSION;
    if (std::abs(pctChange) <= plateauEpsilonPct) return TrendFlagStatus::PLATEAU;
    return TrendFlagStatus::OK;
}

/**
 * @brief Detects exercise trends over a date range using recent session best values.
 *
 * Loads per-session best estimated 1RM data from the repository, groups results
 * by exercise, retains only the most recent minSessionsRequired sessions per
 * exercise in a sliding window, and classifies each exercise's trend using
 * fixed plateau (1%) and regression (3%) thresholds.
 *
 * @param userId               The ID of the user.
 * @param startIsoInclusive    ISO datetime marking the start of the range (inclusive).
 * @param endIsoExclusive      ISO datetime marking the end of the range (exclusive).
 * @param minSessionsRequired  Minimum sessions needed for a valid trend classification (default 6).
 * @return Vector of ExerciseTrendFlag, one per exercise found in the range.
 *
 * @author Adrian Caricari
 */
std::vector<ExerciseTrendFlag> PlateauRegressionService::detect(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive,
    int minSessionsRequired
) {
    // Loads per session best metrics for each exercise from the repository.
    const auto rows = repo_.getExerciseSessionBests(userId, startIsoInclusive, endIsoExclusive);

    // Groups rows by exercise and keeps only the most recent required sessions.
    std::unordered_map<int, std::deque<ExerciseSessionBest>> perExercise;
    std::unordered_map<int, std::string> nameById;

    for (const auto& r : rows) {
        nameById[r.exerciseId] = r.exerciseName;
        auto& dq = perExercise[r.exerciseId];
        dq.push_back(r);
        if ((int)dq.size() > minSessionsRequired) dq.pop_front();
    }

    // Prepares the output list of trend flags.
    std::vector<ExerciseTrendFlag> out;
    out.reserve(perExercise.size());

    // Builds a trend result for each exercise based on its retained session window.
    for (const auto& kv : perExercise) {
        int exerciseId = kv.first;
        const auto& window = kv.second;

        ExerciseTrendFlag flag;
        flag.exerciseId = exerciseId;
        flag.exerciseName = nameById[exerciseId];
        flag.metricName = "estimated_1rm_epley";
        flag.minSessionsRequired = minSessionsRequired;
        flag.sessionsUsed = (int)window.size();

        // Marks exercises with too few sessions as not having enough data
        if ((int)window.size() < minSessionsRequired) {
            flag.status = TrendFlagStatus::NOT_ENOUGH_DATA;
            out.push_back(flag);
            continue;
        }

        // Fills in the comparison window details and metric changes.
        flag.windowStartIso = window.front().performedAtIso;
        flag.windowEndIso   = window.back().performedAtIso;
        flag.firstValue     = window.front().bestMetric;
        flag.lastValue      = window.back().bestMetric;
        flag.delta          = flag.lastValue - flag.firstValue;

        // Classifies the exercise trend using fixed plateau and regression thresholds.
        flag.status = classifyWindow(window, /*plateau*/0.01, /*regression*/0.03);

        // Adds the completed trend flag to the output.
        out.push_back(flag);
    }

    // Returns all detected exercise trend flags.
    return out;
}
