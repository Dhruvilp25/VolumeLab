#include "datetime.hpp"
#include "database.hpp"
#include "template_day.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <algorithm>
#include <stdexcept>

/**
 * @struct ScheduledWorkout
 * @brief Represents a single planned workout instance in the schedule.
 *
 * This struct stores the basic properties of a scheduled workout retrieved
 * from the database.
 * @author Eric Jiang
 */
struct ScheduledWorkout {
    enum StatusCode { PENDING, MISSED, SKIPPED, COMPLETED };

    int scheduledId;
    std::string plannedFor;
    int templateDayId;
    std::string titleSnapshot;
    std::string status;

    int getId() const { return scheduledId; }
    int getTemplateId() const { return templateDayId; }

    StatusCode getStatus() const {
        if (status == "missed") return MISSED;
        if (status == "skipped") return SKIPPED;
        if (status == "completed") return COMPLETED;
        return PENDING;
    }
};

/**
 * @class Scheduler
 * @brief Manages scheduled workouts for a user utilizing the database.
 *
 * This class provides operations to add, retrieve, and manage the cycle
 * of scheduled workouts mapped to the planned_workouts table.
 * @author Eric Jiang
 */
class Scheduler {
private:
    Database* db_ = nullptr;
    int userId = 1;
    int nextInMemoryId_ = 1;
    std::vector<ScheduledWorkout> inMemorySchedule_;

    /**
     * @brief Formats a DateTime object into a standard ISO 8601 string.
     * @author Eric Jiang
     * @param dt The DateTime object to format.
     * @return A string representing the date (YYYY-MM-DD).
     */
    std::string toIsoString(const DateTime& dt) {
        char buf[11];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", dt.year, dt.month, dt.day);
        return std::string(buf);
    }

    DateTime fromIsoString(const std::string& isoDate) const {
        DateTime dt{};
        if (sscanf(isoDate.c_str(), "%d-%d-%d", &dt.year, &dt.month, &dt.day) != 3) {
            throw std::runtime_error("Invalid ISO date");
        }
        return dt;
    }

    bool isDbBacked() const { return db_ != nullptr; }

public:
    Scheduler() = default;

    /**
     * @brief Constructs a new Scheduler linked to a user.
     * @author Eric Jiang
     * @param db A reference to the Database object.
     * @param uid The integer ID of the active user.
     */
    Scheduler(Database& db, int uid) : db_(&db), userId(uid) {}

    /**
     * @brief Assigns a workout template to a specific date and saves it.
     *
     * Validates the template using the TemplateDayService and inserts a
     * new record into the planned_workouts table.
     * @author Eric Jiang
     * @param daySvc A reference to the TemplateDayService for validation.
     * @param templateDayId The ID of the template day being scheduled.
     * @param date The date the workout is scheduled for.
     */
    void assignTemplateToDay(TemplateDayService& daySvc, int templateDayId, DateTime date) {
        if (!isDbBacked()) {
            assignTemplateToDay(templateDayId, date, false);
            return;
        }

        std::string title = "Workout";
        try {
            TemplateDay td = daySvc.getTemplateDayById(templateDayId);
            title = td.name;
        } catch (...) {
            std::cout << "Warning: Template Day not found. Scheduling anyway with generic title.\n";
        }

        std::string dateStr = toIsoString(date);

        const std::string sql = "INSERT INTO planned_workouts (user_id, planned_for, template_day_id, title_snapshot, status) VALUES (?, ?, ?, ?, 'planned');";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, dateStr);
        Database::bindInt(stmt, 3, templateDayId);
        Database::bindText(stmt, 4, title);

        sqlite3_step(stmt);
        db_->finalize(stmt);
    }

    // Backward-compatible in-memory overload used by acceptance test.
    void assignTemplateToDay(int templateDayId, DateTime date, bool /*isRestDay*/) {
        ScheduledWorkout sw;
        sw.scheduledId = nextInMemoryId_++;
        sw.plannedFor = toIsoString(date);
        sw.templateDayId = templateDayId;
        sw.titleSnapshot = "Workout";
        sw.status = "planned";
        inMemorySchedule_.push_back(sw);
    }

    /**
     * @brief Retrieves all relevant workouts scheduled for a specific day.
     *
     * Queries the database for workouts mapped to the given date that are
     * still in the "planned" status.
     * @author Eric Jiang
     * @param today The target DateTime to query.
     * @return A vector of ScheduledWorkout instances for the given day.
     */
    std::vector<ScheduledWorkout> getTodaysWorkout(DateTime today) {
        if (!isDbBacked()) {
            std::vector<ScheduledWorkout> workouts;
            const std::string dateStr = toIsoString(today);
            for (const auto& sw : inMemorySchedule_) {
                if (sw.plannedFor == dateStr && sw.status == "planned") {
                    workouts.push_back(sw);
                }
            }
            return workouts;
        }

        std::vector<ScheduledWorkout> workouts;
        std::string dateStr = toIsoString(today);

        const std::string sql = "SELECT planned_id, planned_for, template_day_id, title_snapshot, status FROM planned_workouts WHERE user_id = ? AND planned_for = ? AND status = 'planned';";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, dateStr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ScheduledWorkout sw;
            sw.scheduledId = Database::colInt(stmt, 0);
            sw.plannedFor = Database::colText(stmt, 1);
            sw.templateDayId = Database::colInt(stmt, 2);
            sw.titleSnapshot = Database::colText(stmt, 3);
            sw.status = Database::colText(stmt, 4);
            workouts.push_back(sw);
        }
        db_->finalize(stmt);
        return workouts;
    }

    /**
     * @brief Retrieves all workouts scheduled for the upcoming week.
     *
     * Uses SQLite date modifiers to fetch workouts from the given start date
     * up to 7 days in the future.
     * @author Eric Jiang
     * @param today The starting DateTime.
     * @return A vector of ScheduledWorkout instances within the timeframe.
     */
    std::vector<ScheduledWorkout> getWeeklyOverview(DateTime today) {
        if (!isDbBacked()) {
            return getWeeklyOverview(today, today);
        }

        std::vector<ScheduledWorkout> workouts;
        std::string startStr = toIsoString(today);

        const std::string sql = "SELECT planned_id, planned_for, template_day_id, title_snapshot, status "
                                "FROM planned_workouts "
                                "WHERE user_id = ? AND planned_for >= ? AND planned_for <= date(?, '+7 days') "
                                "ORDER BY planned_for ASC;";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        Database::bindText(stmt, 2, startStr);
        Database::bindText(stmt, 3, startStr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ScheduledWorkout sw;
            sw.scheduledId = Database::colInt(stmt, 0);
            sw.plannedFor = Database::colText(stmt, 1);
            sw.templateDayId = Database::colInt(stmt, 2);
            sw.titleSnapshot = Database::colText(stmt, 3);
            sw.status = Database::colText(stmt, 4);
            workouts.push_back(sw);
        }
        db_->finalize(stmt);
        return workouts;
    }

    // Backward-compatible overload used by acceptance test.
    std::vector<ScheduledWorkout> getWeeklyOverview(DateTime start, DateTime end) {
        std::vector<ScheduledWorkout> workouts;
        for (const auto& sw : inMemorySchedule_) {
            DateTime planned = fromIsoString(sw.plannedFor);
            if (!(planned < start) && !(end < planned)) {
                workouts.push_back(sw);
            }
        }
        std::sort(workouts.begin(), workouts.end(), [](const ScheduledWorkout& a, const ScheduledWorkout& b) {
            return a.plannedFor < b.plannedFor;
        });
        return workouts;
    }

    // Backward-compatible APIs used by acceptance test.
    void detectMissedWorkouts(DateTime today) {
        for (auto& sw : inMemorySchedule_) {
            const DateTime planned = fromIsoString(sw.plannedFor);
            if (planned < today && sw.status == "planned") {
                sw.status = "missed";
            }
        }
    }

    void skipMissedWorkout(int scheduledId) {
        for (auto& sw : inMemorySchedule_) {
            if (sw.scheduledId == scheduledId && sw.status == "missed") {
                sw.status = "skipped";
                return;
            }
        }
    }

    void moveMissedWorkout(int scheduledId, DateTime newDate) {
        for (auto& sw : inMemorySchedule_) {
            if (sw.scheduledId == scheduledId && sw.status == "missed") {
                sw.plannedFor = toIsoString(newDate);
                sw.status = "planned";
                return;
            }
        }
    }
    /**
     * @brief Erases all scheduled workouts for the current user.
     *
     * Completely clears the user's records from the planned_workouts table.
     * @author Eric Jiang
     */
    void clearSchedule() {
        if (!isDbBacked()) {
            inMemorySchedule_.clear();
            return;
        }

        const std::string sql = "DELETE FROM planned_workouts WHERE user_id = ?;";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        sqlite3_step(stmt);
        db_->finalize(stmt);
    }
};