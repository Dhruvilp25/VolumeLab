#include "graph_renderer.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

// Author: Adrian Caricari

/**
 * @brief Builds a labelled horizontal ASCII bar chart from weekly trend data.
 *
 * Iterates through each week, extracts a numeric value via valueExtractor,
 * determines the global maximum for scaling, and renders proportional bars
 * using the '#' character. Each row is labelled with the week-start date
 * and the raw numeric value followed by unitLabel.
 *
 * @param title           Chart heading printed on the first output line.
 * @param weeks           Weekly data rows in chronological order.
 * @param valueExtractor  Function pointer that returns a double from a WeeklyTrendRow.
 * @param unitLabel       Text appended after the numeric value on each row.
 * @param maxBarWidth     The character width of the longest bar.
 * @return A ready-to-print multi-line string containing the complete chart.
 */
std::string GraphRenderer::renderBarChart(
    const std::string& title,
    const std::vector<WeeklyTrendRow>& weeks,
    double (*valueExtractor)(const WeeklyTrendRow&),
    const std::string& unitLabel,
    int maxBarWidth
) {
    std::ostringstream out;

    // Prints the chart title and an underline separator.
    out << "\n  " << title << "\n";
    out << "  " << std::string(title.size(), '-') << "\n";

    // Handles the empty data case with a friendly message.
    if (weeks.empty()) {
        out << "  (no data to display)\n";
        return out.str();
    }

    // Finds the maximum value across all weeks for bar scaling.
    double maxVal = 0.0;
    for (const auto& w : weeks) {
        double v = valueExtractor(w);
        if (v > maxVal) maxVal = v;
    }

    // Renders one row per week with a proportional bar.
    for (const auto& w : weeks) {
        double v = valueExtractor(w);

        // Calculates bar length proportional to the maximum value.
        int barLen = 0;
        if (maxVal > 0.0) {
            barLen = static_cast<int>(std::round((v / maxVal) * maxBarWidth));
        }

        // Formats the row: date label, bar, and numeric value.
        out << "  " << w.weekStart << " |";
        out << std::string(static_cast<size_t>(barLen), '#');
        out << " " << std::fixed << std::setprecision(0) << v << " " << unitLabel << "\n";
    }

    out << "\n";
    return out.str();
}

/**
 * @brief Renders a weekly total volume bar chart.
 *
 * Extracts totalVolume (reps x weight) from each WeeklyTrendRow and
 * delegates to renderBarChart with an appropriate title and unit label.
 *
 * @param weeks       Vector of weekly trend rows sorted by weekStart.
 * @param maxBarWidth Maximum character width for the longest bar.
 * @return Multi-line chart string.
 */
std::string GraphRenderer::renderWeeklyVolume(
    const std::vector<WeeklyTrendRow>& weeks,
    int maxBarWidth
) {
    return renderBarChart(
        "Weekly Volume (reps x weight)",
        weeks,
        [](const WeeklyTrendRow& w) -> double { return w.totalVolume; },
        "vol",
        maxBarWidth
    );
}

/**
 * @brief Renders a weekly workout frequency bar chart.
 *
 * Extracts workoutCount from each WeeklyTrendRow.
 *
 * @param weeks       Vector of weekly trend rows sorted by weekStart.
 * @param maxBarWidth Maximum character width for the longest bar.
 * @return Multi-line chart string.
 */
std::string GraphRenderer::renderWeeklyFrequency(
    const std::vector<WeeklyTrendRow>& weeks,
    int maxBarWidth
) {
    return renderBarChart(
        "Weekly Workout Frequency",
        weeks,
        [](const WeeklyTrendRow& w) -> double { return static_cast<double>(w.workoutCount); },
        "workouts",
        maxBarWidth
    );
}

/**
 * @brief Renders a weekly total sets bar chart.
 *
 * Extracts totalSets from each WeeklyTrendRow.
 *
 * @param weeks       Vector of weekly trend rows sorted by weekStart.
 * @param maxBarWidth Maximum character width for the longest bar.
 * @return Multi-line chart string.
 */
std::string GraphRenderer::renderWeeklySets(
    const std::vector<WeeklyTrendRow>& weeks,
    int maxBarWidth
) {
    return renderBarChart(
        "Weekly Total Sets",
        weeks,
        [](const WeeklyTrendRow& w) -> double { return static_cast<double>(w.totalSets); },
        "sets",
        maxBarWidth
    );
}

/**
 * @brief Renders a weekly total reps bar chart.
 *
 * Extracts totalReps from each WeeklyTrendRow.
 *
 * @param weeks       Vector of weekly trend rows sorted by weekStart.
 * @param maxBarWidth Maximum character width for the longest bar.
 * @return Multi-line chart string.
 */
std::string GraphRenderer::renderWeeklyReps(
    const std::vector<WeeklyTrendRow>& weeks,
    int maxBarWidth
) {
    return renderBarChart(
        "Weekly Total Reps",
        weeks,
        [](const WeeklyTrendRow& w) -> double { return static_cast<double>(w.totalReps); },
        "reps",
        maxBarWidth
    );
}
