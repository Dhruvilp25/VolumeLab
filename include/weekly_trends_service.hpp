#pragma once
#include <vector>
#include <string>
#include "weekly_trends_repository.hpp"

/**
 * @brief Service layer that forwards weekly trend requests to the repository.
 *
 * Acts as a thin pass-through between the UI and the WeeklyTrendsRepository,
 * keeping SQL concerns out of the presentation layer.
 *
 * @author Adrian Caricari
 */
class WeeklyTrendsService {
public:
    /**
     * @brief Constructs the service with its required repository.
     * @param repo Reference to the WeeklyTrendsRepository used for data access.
     */
    explicit WeeklyTrendsService(WeeklyTrendsRepository& repo) : repo_(repo) {}

    /**
     * @brief Retrieves weekly workout trend rows for a user within a date range.
     *
     * Delegates directly to the repository without additional business logic.
     *
     * @param userId             The ID of the user.
     * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
     * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
     * @return Vector of WeeklyTrendRow sorted by weekStart ascending.
     */
    std::vector<WeeklyTrendRow> getWeeklyTrends(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    ) {
        return repo_.getWeeklyTrends(userId, startIsoInclusive, endIsoExclusive);
    }

private:
    WeeklyTrendsRepository& repo_;  ///< Reference to the weekly trends repository.
};
