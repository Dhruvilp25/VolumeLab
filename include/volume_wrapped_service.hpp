#pragma once
#include <string>
#include "analytics_dashboard_service.hpp"
#include "volume_wrapped.hpp"
#include "volume_wrapped_repository.hpp"
#include "database.hpp"

// Author: Adrian Caricari

// Service responsible for building the full Volume Wrapped report.
class VolumeWrappedService {
public:
    // Initializes the service with its required dashboard, repository, and database dependencies.
    VolumeWrappedService(
        AnalyticsDashboardService& dashboardService,
        VolumeWrappedRepository& repo,
        Database& db
    ) : dashboardService_(dashboardService), repo_(repo), db_(db) {}

    // Builds and returns a Volume Wrapped report for the given user and date range.
    VolumeWrappedReport getVolumeWrapped(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

private:
    // Computes how many calendar weeks are covered by the given date range.
    int computeCalendarWeeksInRange(
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

    // Reference to the dashboard service used for shared analytics data.
    AnalyticsDashboardService& dashboardService_;

    // Reference to the repository used for Volume Wrapped-specific queries.
    VolumeWrappedRepository& repo_;

    // Reference to the database used for internal date calculations.
    Database& db_;
};