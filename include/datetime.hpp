/**
 * @file DateTime.h
 * @brief Definition for the DateTime structure used across the app.
 * @author Eric Jiang
 */

#ifndef DATETIME
#define DATETIME

#include <ctime>

/**
 * @struct DateTime
 * @brief A structure to represent a specific calendar date.
 * * Note: Consider migrating to <chrono> and standard date libraries 
 * if modern C++ features are available in the project.
 */
struct DateTime {
    int year;  /**< The calendar year */
    int month; /**< The month of the year (1 through 12) */
    int day;   /**< The day of the month (1 through 31) */

    /**
     * @brief Compares two DateTime objects for equality.
     * @param other The DateTime object to compare against.
     * @return True if the year, month, and day are exactly the same, false otherwise.
     */
    bool operator==(const DateTime& other) const {
        return year == other.year && month == other.month && day == other.day;
    }

    /**
     * @brief Checks if this DateTime occurs strictly before another DateTime.
     * @param other The DateTime object to compare against.
     * @return True if this date is chronologically earlier than the other date, false otherwise.
     */
    bool operator<(const DateTime& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }

    /**
     * @brief Retrieves the current local date.
     * @return A DateTime object populated with today's date based on the system clock.
     */
    static DateTime now() {
        std::time_t t = std::time(nullptr);
        std::tm* localTime = std::localtime(&t);
        
        DateTime currentDate;
        currentDate.year = localTime->tm_year + 1900;
        currentDate.month = localTime->tm_mon + 1;
        currentDate.day = localTime->tm_mday;
        
        return currentDate;
    }
};

#endif