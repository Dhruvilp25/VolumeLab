#pragma once
#include <string>
#include <vector>
#include "weekly_trends.hpp"

/**
 * @brief Renders ASCII bar-chart graphs for analytics metrics in the terminal.
 *
 * Provides static methods to turn weekly trend data into human-readable
 * text-based bar charts that can be printed directly to std::cout.
 * Each chart auto-scales to a configurable maximum bar width and labels
 * every row with its week-start date and numeric value.
 *
 * @author Adrian Caricari
 */
class GraphRenderer {
public:
    /**
     * @brief Renders a horizontal bar chart of weekly total volume (reps x weight).
     *
     * Each bar represents one week. Bars are scaled so the largest value fills
     * maxBarWidth characters. Zero-value weeks still show a label line.
     *
     * @param weeks   Vector of WeeklyTrendRow, assumed sorted by weekStart ascending.
     * @param maxBarWidth  Maximum number of block characters for the longest bar.
     * @return A multi-line string containing the full chart ready for printing.
     */
    static std::string renderWeeklyVolume(
        const std::vector<WeeklyTrendRow>& weeks,
        int maxBarWidth = 40
    );

    /**
     * @brief Renders a horizontal bar chart of weekly workout count (frequency).
     *
     * Each bar represents the number of workouts in that week.
     *
     * @param weeks   Vector of WeeklyTrendRow, assumed sorted by weekStart ascending.
     * @param maxBarWidth  Maximum number of block characters for the longest bar.
     * @return A multi-line string containing the full chart ready for printing.
     */
    static std::string renderWeeklyFrequency(
        const std::vector<WeeklyTrendRow>& weeks,
        int maxBarWidth = 40
    );

    /**
     * @brief Renders a horizontal bar chart of weekly total sets.
     *
     * @param weeks   Vector of WeeklyTrendRow, assumed sorted by weekStart ascending.
     * @param maxBarWidth  Maximum number of block characters for the longest bar.
     * @return A multi-line string containing the full chart ready for printing.
     */
    static std::string renderWeeklySets(
        const std::vector<WeeklyTrendRow>& weeks,
        int maxBarWidth = 40
    );

    /**
     * @brief Renders a horizontal bar chart of weekly total reps.
     *
     * @param weeks   Vector of WeeklyTrendRow, assumed sorted by weekStart ascending.
     * @param maxBarWidth  Maximum number of block characters for the longest bar.
     * @return A multi-line string containing the full chart ready for printing.
     */
    static std::string renderWeeklyReps(
        const std::vector<WeeklyTrendRow>& weeks,
        int maxBarWidth = 40
    );

private:
    /**
     * @brief Generic helper that builds a labelled horizontal bar chart.
     *
     * @param title       Chart title printed on the first line.
     * @param weeks       Source data rows.
     * @param valueExtractor  Function that pulls the numeric value from a row.
     * @param unitLabel   Suffix printed after each numeric value (e.g. "lbs", "sets").
     * @param maxBarWidth Maximum bar length in characters.
     * @return Formatted multi-line string.
     */
    static std::string renderBarChart(
        const std::string& title,
        const std::vector<WeeklyTrendRow>& weeks,
        double (*valueExtractor)(const WeeklyTrendRow&),
        const std::string& unitLabel,
        int maxBarWidth
    );
};
