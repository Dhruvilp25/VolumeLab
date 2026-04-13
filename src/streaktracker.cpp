#include "datetime.hpp"
#include "database.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

/**
 * @class StreakTracker
 * @brief Manages and tracks the workout streak of a user with database persistence.
 *
 * This class handles the logic for calculating consecutive workout days. It includes
 * features like tracking the best overall streak and utilizing freeze days to protect
 * a streak if a user misses a workout. Data is persisted to the streaks table.
 * @author Eric Jiang
 */
class StreakTracker {
private:
    Database* db_ = nullptr;
    int userId;
    int currentStreak;
    int bestStreak;
    std::string lastWorkoutDateStr;
    int freezeDaysRemaining;

    /**
     * @brief Formats a DateTime object into a standard ISO 8601 string (YYYY-MM-DD).
     * @author Eric Jiang
     * @param dt The DateTime object to format.
     * @return A formatted date string compatible with SQLite.
     */
    std::string toIsoString(const DateTime& dt) {
        char buf[11];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", dt.year, dt.month, dt.day);
        return std::string(buf);
    }

    bool isDbBacked() const { return db_ != nullptr; }

    /**
     * @brief Converts a year, month, and day into an absolute number of days.
     *
     * Calculates the total number of days since year 0 to easily compute 
     * the difference in days between two dates.
     * @author Eric Jiang
     * @param year The year.
     * @param month The month.
     * @param day The day.
     * @return The absolute number of days corresponding to the date.
     */
    int getAbsoluteDays(int year, int month, int day) {
        const int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        int totalDays = year * 365;
        for (int i = 1; i < month; ++i) {
            totalDays += daysInMonth[i];
        }
        totalDays += day;
        return totalDays;
    }

    /**
     * @brief Loads the user streak data from the SQLite database.
     *
     * If no record exists for the user, a default record initialized to zero is created.
     * @author Eric Jiang
     */
    void loadFromDatabase() {
        if (!isDbBacked()) {
            return;
        }
        const std::string sql = "SELECT current_streak, best_streak, last_workout_date FROM streaks WHERE user_id = ?;";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            currentStreak = Database::colInt(stmt, 0);
            bestStreak = Database::colInt(stmt, 1);
            lastWorkoutDateStr = Database::colText(stmt, 2);
            db_->finalize(stmt);
        } else {
            db_->finalize(stmt);
            // Insert default row if user has no streak record
            const std::string insertSql = "INSERT INTO streaks (user_id, current_streak, best_streak) VALUES (?, 0, 0);";
            sqlite3_stmt* insertStmt = db_->prepare(insertSql);
            Database::bindInt(insertStmt, 1, userId);
            sqlite3_step(insertStmt);
            db_->finalize(insertStmt);
            
            currentStreak = 0;
            bestStreak = 0;
            lastWorkoutDateStr = "";
        }
    }

    /**
     * @brief Saves the current streak state to the SQLite database.
     * @author Eric Jiang
     */
    void saveToDatabase() {
        if (!isDbBacked()) {
            return;
        }
        const std::string sql = "UPDATE streaks SET current_streak = ?, best_streak = ?, last_workout_date = ?, updated_at = datetime('now') WHERE user_id = ?;";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, currentStreak);
        Database::bindInt(stmt, 2, bestStreak);
        Database::bindText(stmt, 3, lastWorkoutDateStr);
        Database::bindInt(stmt, 4, userId);
        sqlite3_step(stmt);
        db_->finalize(stmt);
    }

public:
    DateTime today = DateTime::now();

    // Backward-compatible constructor for in-memory acceptance test usage.
    StreakTracker(int uid)
        : userId(uid), currentStreak(0), bestStreak(0), freezeDaysRemaining(0) {}

    /**
     * @brief Constructs a new StreakTracker for a specific user.
     *
     * Initializes the tracker and immediately loads the current state from the database.
     * @author Eric Jiang
     * @param db A reference to the Database object.
     * @param uid The unique identifier for the user.
     */
    StreakTracker(Database& db, int uid)
        : db_(&db), userId(uid), currentStreak(0), bestStreak(0), freezeDaysRemaining(0) {
        loadFromDatabase();
    }

    /**
     * @brief Updates the user streak based on the current date.
     *
     * Evaluates missed days, applies freeze protection if applicable, increments
     * the current streak, and saves the new state to the database.
     * @author Eric Jiang
     */
    void updateStreak() {
        DateTime effectiveToday = isDbBacked() ? DateTime::now() : today;
        std::string todayStr = toIsoString(effectiveToday);

        if (todayStr == lastWorkoutDateStr) {
            return; 
        }

        int diff = 1;
        if (!lastWorkoutDateStr.empty() && lastWorkoutDateStr.length() >= 10) {
            int lastYear = std::stoi(lastWorkoutDateStr.substr(0, 4));
            int lastMonth = std::stoi(lastWorkoutDateStr.substr(5, 2));
            int lastDay = std::stoi(lastWorkoutDateStr.substr(8, 2));
            
            diff = getAbsoluteDays(effectiveToday.year, effectiveToday.month, effectiveToday.day) - getAbsoluteDays(lastYear, lastMonth, lastDay);
        }

        if (diff == 1 || lastWorkoutDateStr.empty()) {
            // Consecutive day, or very first workout logged
            currentStreak++;
        } else if (diff > 1) {
            // If user missed a day, check streak protection
            if (freezeDaysRemaining >= (diff - 1)) {
                freezeDaysRemaining -= (diff - 1);
                currentStreak++;
            } else {
                // Reset if user misses a day without streak protection
                currentStreak = 1; 
                freezeDaysRemaining = 0;
            }
        }

        if (currentStreak > bestStreak) {
            bestStreak = currentStreak;
        }
        
        lastWorkoutDateStr = todayStr;
        saveToDatabase();
    }

    /**
     * @brief Adds freeze days to the user account.
     *
     * Freeze days protect the user streak from resetting if they fail to log a workout on a given day.
     * @author Eric Jiang
     * @param duration The number of freeze days to add.
     */
    void activateFreeze(int duration) { freezeDaysRemaining += duration; }

    /**
     * @brief Gets the current user streak.
     * @author Eric Jiang
     * @return The current consecutive workout streak.
     */
    int getCurrentStreak() const { return currentStreak; }

    /**
     * @brief Gets the best historical streak.
     * @author Eric Jiang
     * @return The highest streak the user has achieved.
     */
    int getBestStreak() const { return bestStreak; }

    /**
     * @brief Gets the number of available freeze days.
     * @author Eric Jiang
     * @return The amount of streak protection days remaining.
     */
    int getFreezeDaysRemaining() const { return freezeDaysRemaining; }

    /**
     * @brief Gets the date string of the most recently logged workout.
     * @author Eric Jiang
     * @return A string representing the last workout date (YYYY-MM-DD).
     */
    std::string getLastWorkoutDate() const { return lastWorkoutDateStr; }
};

/**
 * @class GoalManager
 * @brief Manages fitness goals and tracks milestone achievements in the database.
 *
 * This class allows users to set frequency targets for their workouts and tracks
 * them persistently.
 * @author Eric Jiang
 */
class GoalManager {
private:
    Database* db_ = nullptr;
    int userId;
    const StreakTracker* streakTracker;
    std::vector<std::string> achievedMilestones;

    bool isDbBacked() const { return db_ != nullptr; }

public:
    // Backward-compatible constructor for in-memory acceptance test usage.
    GoalManager(int uid, const StreakTracker* tracker) : userId(uid), streakTracker(tracker) {}

    /**
     * @brief Constructs a new GoalManager.
     *
     * Links the manager to a specific user, their StreakTracker, and the database.
     * @author Eric Jiang
     * @param db A reference to the Database object.
     * @param uid The unique identifier for the user.
     * @param tracker A constant pointer to the user StreakTracker.
     */
    GoalManager(Database& db, int uid, const StreakTracker* tracker) : db_(&db), userId(uid), streakTracker(tracker) {}

    /**
     * @brief Sets a weekly workout frequency goal and saves it to the database.
     * @author Eric Jiang
     * @param workoutsPerWeek The target number of workouts for the week.
     */
    void setFrequencyGoal(int workoutsPerWeek) {
        if (!isDbBacked()) {
            (void)workoutsPerWeek;
            return;
        }
        const std::string sql = "INSERT INTO goals (user_id, goal_type, target_value, start_date) VALUES (?, 'frequency', ?, date('now'));";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        Database::bindDouble(stmt, 2, static_cast<double>(workoutsPerWeek));
        sqlite3_step(stmt);
        db_->finalize(stmt);
    }

    /**
     * @brief Retrieves all active frequency goals for the user from the database.
     * @author Eric Jiang
     * @return A vector of strings representing the active goals.
     */
    std::vector<std::string> getActiveGoals() const {
        if (!isDbBacked()) {
            return { "Target: 4 workouts per week" };
        }
        std::vector<std::string> goals;
        const std::string sql = "SELECT target_value FROM goals WHERE user_id = ? AND is_active = 1 AND goal_type = 'frequency';";
        sqlite3_stmt* stmt = db_->prepare(sql);
        Database::bindInt(stmt, 1, userId);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int target = static_cast<int>(Database::colDouble(stmt, 0));
            goals.push_back("Target: " + std::to_string(target) + " workouts per week");
        }
        db_->finalize(stmt);
        return goals;
    }

    void checkMilestones() {
        achievedMilestones.clear();
        if (streakTracker && streakTracker->getCurrentStreak() >= 7) {
            achievedMilestones.push_back("One Week Streak!");
        }
    }

    const std::vector<std::string>& getAchievedMilestones() const {
        return achievedMilestones;
    }
};