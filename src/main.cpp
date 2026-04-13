/**
 * @file main.cpp
 * @brief Console presentation layer for VolumeLab: menus, login, planner, goals, and analytics views.
 * @details
 * Implements the interactive user interface as text-driven screens. It connects existing
 * domain services (accounts, programs, analytics) to stdin/stdout. The Planner uses @c Scheduler
 * (see @c scheduler.cpp); Goals use @c StreakTracker and @c GoalManager (see @c streaktracker.cpp).
 * Those implementation files are included here to keep a single UI translation unit for this build.
 *
 * @author Dhruvil Patel
 */

#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "database.hpp"
#include "user_repository.hpp"
#include "user_account_service.hpp"
#include "program.hpp"
#include "template_day.hpp"
#include "template_exercise.hpp"
#include "exercise_repository.hpp"
#include "analytics_dashboard_service.hpp"
#include "analytics_dashboard_repository.hpp"
#include "weekly_trends_repository.hpp"
#include "weekly_trends_service.hpp"
#include "workout_session_repository.hpp"
#include "workout_session_service.hpp"
#include "scheduler.cpp"
#include "streaktracker.cpp"

/**
 * @brief Signed-in navigation sections shown in the main menu.
 * @details Maps to menu options 1–7; option 0 is log out handled separately.
 */
enum class Section { Dashboard, Programs, Workout, History, Planner, Goals, Analytics };

/**
 * @brief Clears the terminal screen for a cleaner menu display.
 * @author Dhruvil Patel
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Waits for the user to press Enter before continuing.
 * @author Dhruvil Patel
 */
void pause() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * @brief Reads one line of text from standard input after displaying a prompt.
 * @author Dhruvil Patel
 * @param prompt Text printed before the cursor (e.g. "Username: ").
 * @return The line entered by the user, without the trailing newline.
 */
std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

/**
 * @brief Reads a single integer from standard input after displaying a prompt.
 * @details Clears the input error state and remainder of the line if parsing fails.
 * @author Dhruvil Patel
 * @param prompt Text printed before input (e.g. "Choice: ").
 * @return The parsed integer, or @c -1 if the input was not a valid integer.
 */
int readInt(const std::string& prompt) {
    std::cout << prompt;
    int val;
    std::cin >> val;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return val;
}

double readDouble(const std::string& prompt) {
    std::cout << prompt;
    double val;
    std::cin >> val;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return val;
}

static std::string nowSqliteTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * @brief Shows the login or register screen until authentication succeeds or the user exits.
 * @details On success, writes the active user id and username into the output parameters.
 * @author Dhruvil Patel
 * @param userSvc Service used to validate credentials and register new accounts.
 * @param[out] currentUserId Set to the authenticated user’s id on success.
 * @param[out] currentUsername Set to the authenticated user’s name on success.
 */
void showLoginScreen(UserAccountService& userSvc, int& currentUserId, std::string& currentUsername) {
    while (true) {
        clearScreen();
        std::cout << "=== VolumeLab ===\n\n";
        std::cout << "1. Log in\n2. Register\n3. Exit\n\n";
        int choice = readInt("Choice: ");
        if (choice == 3) exit(0);
        if (choice != 1 && choice != 2) continue;

        std::string uname = readLine("Username: ");
        std::string pwd = readLine("Password: ");

        if (choice == 1) {
            LoginResult r = userSvc.login(uname, pwd);
            if (r.success) {
                currentUserId = r.userId;
                currentUsername = r.username;
                return;
            }
            std::cout << "\nError: " << r.errorMessage << "\n";
        } else {
            RegisterResult r = userSvc.registerUser(uname, pwd);
            if (r.success) {
                currentUserId = r.userId;
                currentUsername = r.username;
                std::cout << "\nAccount created! Logging you in.\n";
                pause();
                return;
            }
            std::cout << "\nError: " << r.errorMessage << "\n";
        }
        pause();
    }
}

/**
 * @brief Prints the numbered navigation menu and highlights the current section when applicable.
 * @author Dhruvil Patel
 * @param current The section currently selected (used only for the “[current]” marker).
 */
void showMainMenu(Section& current) {
    std::cout << "\n--- Navigation ---\n";
    const char* sections[] = { "Dashboard", "Programs", "Workout", "History", "Planner", "Goals", "Analytics" };
    for (int i = 0; i < 7; ++i) {
        std::cout << (i + 1) << ". " << sections[i];
        if (static_cast<int>(current) == i) std::cout << " [current]";
        std::cout << "\n";
    }
    std::cout << "0. Log out\n";
}

/**
 * @brief Displays a simple dashboard with the user’s program count and names.
 * @author Dhruvil Patel
 * @param userId Active user id.
 * @param username Display name for the header.
 * @param progSvc Service used to list programs.
 */
void showDashboard(int userId, const std::string& username, ProgramService& progSvc) {
    clearScreen();
    std::cout << "=== Dashboard ===\n";
    std::cout << "Logged in as: " << username << "\n\n";
    auto programs = progSvc.getProgramsByUser(userId);
    std::cout << "Your programs: " << programs.size() << "\n";
    for (const auto& p : programs) {
        std::cout << "  - " << p.name << " (created " << p.createdAt << ")\n";
    }
    if (programs.empty()) {
        std::cout << "  No programs yet. Go to Programs to create one.\n";
    }
}

static void showTemplateDayBuilder(
    int userId,
    const Program& program,
    const TemplateDaySummary& day,
    ExerciseRepository& exRepo,
    TemplateExerciseService& templateExerciseSvc
) {
    while (true) {
        clearScreen();
        std::cout << "=== Template Builder ===\n\n";
        std::cout << "Program: " << program.name << "\n";
        std::cout << "Template day: " << day.name << "\n\n";

        auto exercises = templateExerciseSvc.getExercisesWithDetails(day.templateDayId);
        if (exercises.empty()) {
            std::cout << "No exercises yet.\n\n";
        } else {
            std::cout << "Exercises:\n";
            for (size_t i = 0; i < exercises.size(); ++i) {
                const auto& e = exercises[i];
                std::cout << "  " << (i + 1) << ". " << e.exerciseName;
                if (e.targetSets > 0 || !e.targetReps.empty() || e.targetWeightKg > 0) {
                    std::cout << " — ";
                    if (e.targetSets > 0) std::cout << e.targetSets << " sets";
                    if (!e.targetReps.empty()) std::cout << ", reps " << e.targetReps;
                    std::cout << ", " << e.targetWeightKg << " kg";
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        std::cout << "1. Add exercise\n";
        std::cout << "2. Back\n\n";

        int choice = readInt("Choice: ");
        if (choice == 2) return;
        if (choice != 1) continue;

        std::string exName = readLine("Exercise name: ");
        if (exName.empty()) continue;

        int targetSets = readInt("Target sets (0 for none): ");
        if (targetSets < 0) targetSets = 0;

        std::string targetReps = readLine("Target reps (e.g. 6-8, blank for none): ");
        double targetWeight = readDouble("Target weight kg (0 allowed): ");
        if (targetWeight < 0) targetWeight = 0;

        try {
            int exId = exRepo.findOrCreate(userId, exName);
            if (templateExerciseSvc.existsInTemplateDay(day.templateDayId, exId)) {
                std::cout << "That exercise is already in this template.\n";
                pause();
                continue;
            }

            if (targetSets > 0 || !targetReps.empty() || targetWeight > 0) {
                templateExerciseSvc.addExerciseWithTargets(day.templateDayId, exId, targetSets, targetReps, targetWeight);
            } else {
                templateExerciseSvc.addExerciseToTemplateDay(day.templateDayId, exId);
            }

            std::cout << "Exercise added!\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
        pause();
    }
}

static void showProgramDetailsScreen(
    int userId,
    const Program& program,
    TemplateDayService& daySvc,
    ExerciseRepository& exRepo,
    TemplateExerciseService& templateExerciseSvc
) {
    while (true) {
        clearScreen();
        std::cout << "=== " << program.name << " ===\n\n";

        auto days = daySvc.getTemplateDaysWithSummary(program.programId);
        if (days.empty()) {
            std::cout << "No template days yet.\n\n";
        } else {
            std::cout << "Template days:\n";
            for (size_t i = 0; i < days.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << days[i].name
                          << " (" << days[i].exerciseCount << " exercises, " << days[i].totalSets << " sets)\n";
            }
            std::cout << "\n";
        }

        std::cout << "1. Add template day\n";
        if (!days.empty()) {
            std::cout << "2. Open template day\n";
            std::cout << "3. Rename template day\n";
            std::cout << "4. Delete template day\n";
            std::cout << "5. Back\n\n";
        } else {
            std::cout << "2. Back\n\n";
        }

        int choice = readInt("Choice: ");
        if (days.empty()) {
            if (choice == 2) return;
        } else if (choice == 5) {
            return;
        }

        if (choice == 1) {
            std::string dayName = readLine("Template day name (e.g. Push Day): ");
            if (dayName.empty()) continue;
            try {
                daySvc.createTemplateDay(program.programId, dayName);
                std::cout << "Template day added!\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
            pause();
        } else if (choice >= 2 && choice <= 4 && !days.empty()) {
            int idx = readInt("Template day number: ");
            if (idx < 1 || idx > static_cast<int>(days.size())) continue;
            auto selectedDay = days[idx - 1];

            if (choice == 2) {
                showTemplateDayBuilder(userId, program, selectedDay, exRepo, templateExerciseSvc);
            } else if (choice == 3) {
                std::string newName = readLine("New name: ");
                if (newName.empty()) continue;
                try {
                    daySvc.renameTemplateDay(selectedDay.templateDayId, newName);
                    std::cout << "Renamed!\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                }
                pause();
            } else if (choice == 4) {
                std::cout << "Delete '" << selectedDay.name << "'? (1=yes / 2=no)\n";
                int confirm = readInt("Choice: ");
                if (confirm == 1) {
                    try {
                        daySvc.deleteTemplateDay(selectedDay.templateDayId);
                        std::cout << "Deleted.\n";
                    } catch (const std::exception& e) {
                        std::cout << "Error: " << e.what() << "\n";
                    }
                    pause();
                }
            }
        }
    }
}

/**
 * @brief Interactive screen to create programs and inspect template days.
 * @author Dhruvil Patel
 * @param userId Active user id.
 * @param progSvc Program creation and listing.
 * @param daySvc Template day creation and summaries for a program.
 */
void showProgramsScreen(int userId, ProgramService& progSvc, TemplateDayService& daySvc,
                        ExerciseRepository& exRepo, TemplateExerciseService& templateExerciseSvc) {
    while (true) {
        clearScreen();
        std::cout << "=== Programs ===\n\n";
        auto programs = progSvc.getProgramsByUser(userId);

        if (programs.empty()) {
            std::cout << "No programs yet.\n";
            std::cout << "Create one to get started.\n\n";
        } else {
            std::cout << "Your programs:\n";
            for (size_t i = 0; i < programs.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << programs[i].name << " (created " << programs[i].createdAt << ")\n";
            }
            std::cout << "\n";
        }

        std::cout << "1. Create program\n";
        if (!programs.empty()) {
            std::cout << "2. Open program\n";
            std::cout << "3. Back to menu\n";
        } else {
            std::cout << "2. Back to menu\n";
        }

        int choice = readInt("Choice: ");
        if (choice == 1) {
            std::string name = readLine("Program name: ");
            if (!name.empty()) {
                try {
                    progSvc.createProgram(userId, name);
                    std::cout << "Program created!\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                }
                pause();
            }
        } else if (choice == 2 && !programs.empty()) {
            int idx = readInt("Program number: ");
            if (idx >= 1 && idx <= static_cast<int>(programs.size())) {
                Program p = programs[idx - 1];
                showProgramDetailsScreen(userId, p, daySvc, exRepo, templateExerciseSvc);
            }
        } else if (choice == 3 || (choice == 2 && programs.empty())) {
            return;
        }
    }
}

void showWorkoutScreen(
    int userId,
    const std::string& username,
    ProgramService& progSvc,
    TemplateDayService& daySvc,
    ExerciseRepository& exRepo,
    TemplateExerciseService& templateExerciseSvc,
    WorkoutSessionService& workoutSvc
) {
    while (true) {
        clearScreen();
        std::cout << "=== Workout ===\n";
        std::cout << "Logged in as: " << username << "\n\n";

        auto programs = progSvc.getProgramsByUser(userId);
        if (programs.empty()) {
            std::cout << "No programs found.\n";
            std::cout << "Create a program first under Programs.\n";
            pause();
            return;
        }

        std::cout << "Select a program:\n";
        for (size_t i = 0; i < programs.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << programs[i].name << "\n";
        }
        std::cout << "  0. Back\n\n";

        int pIdx = readInt("Choice: ");
        if (pIdx == 0) return;
        if (pIdx < 1 || pIdx > static_cast<int>(programs.size())) continue;

        Program p = programs[pIdx - 1];
        auto days = daySvc.getTemplateDaysWithSummary(p.programId);
        if (days.empty()) {
            std::cout << "\nNo template days in this program.\n";
            std::cout << "Add a template day under Programs.\n";
            pause();
            continue;
        }

        clearScreen();
        std::cout << "=== Start Workout ===\n\n";
        std::cout << "Program: " << p.name << "\n\n";
        std::cout << "Select a template day:\n";
        for (size_t i = 0; i < days.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << days[i].name << "\n";
        }
        std::cout << "  0. Back\n\n";

        int dIdx = readInt("Choice: ");
        if (dIdx == 0) continue;
        if (dIdx < 1 || dIdx > static_cast<int>(days.size())) continue;

        auto selectedDay = days[dIdx - 1];
        auto templateExercises = templateExerciseSvc.getExercisesWithDetails(selectedDay.templateDayId);
        if (templateExercises.empty()) {
            std::cout << "\nThis template has no exercises.\n";
            std::cout << "1. Open template builder to add exercises\n";
            std::cout << "2. Back\n\n";
            int emptyChoice = readInt("Choice: ");
            if (emptyChoice == 1) {
                showTemplateDayBuilder(userId, p, selectedDay, exRepo, templateExerciseSvc);
            }
            continue;
        }

        WorkoutSession session;
        session.startSession(userId);
        session.performedAt = nowSqliteTimestamp();
        session.sourceLabel = selectedDay.name;
        session.durationSeconds = 0;

        for (const auto& te : templateExercises) {
            session.addExercise(te.exerciseId, te.exerciseName);
        }

        auto startedAt = std::chrono::steady_clock::now();

        // Log sets exercise-by-exercise
        for (auto& ex : session.exercises) {
            clearScreen();
            std::cout << "=== Active Workout ===\n\n";
            std::cout << "Template: " << selectedDay.name << "\n";
            std::cout << "Exercise: " << ex.exerciseNameSnapshot << "\n\n";

            // Find target prescription (if present)
            int targetSets = 0;
            std::string targetReps;
            double targetWeight = 0;
            for (const auto& te : templateExercises) {
                if (te.exerciseId == ex.exerciseId) {
                    targetSets = te.targetSets;
                    targetReps = te.targetReps;
                    targetWeight = te.targetWeightKg;
                    break;
                }
            }

            if (targetSets > 0 || !targetReps.empty() || targetWeight > 0) {
                std::cout << "Target: ";
                if (targetSets > 0) std::cout << targetSets << " sets";
                if (!targetReps.empty()) std::cout << ", reps " << targetReps;
                if (targetWeight > 0) std::cout << ", weight " << targetWeight << " kg";
                std::cout << "\n\n";
            }

            int setsToLog = readInt("How many sets did you do? ");
            if (setsToLog <= 0) {
                std::cout << "You must log at least 1 set per exercise.\n";
                pause();
                setsToLog = 1;
            }

            for (int s = 1; s <= setsToLog; ++s) {
                int reps = readInt("  Set " + std::to_string(s) + " reps: ");
                double weight = readDouble("  Set " + std::to_string(s) + " weight (kg): ");
                if (reps < 0 || weight < 0) {
                    std::cout << "  Invalid values. Reps/weight must be non-negative.\n";
                    --s;
                    continue;
                }
                workoutSvc.logSet(session, ex.exerciseId, reps, weight);
            }
        }

        clearScreen();
        std::cout << "=== Finish Workout ===\n\n";
        std::cout << "Template: " << selectedDay.name << "\n";
        std::cout << "Exercises: " << session.exercises.size() << "\n";
        std::cout << "Total volume: " << session.calculateTotalVolume() << " kg\n\n";
        std::cout << "1. Save workout\n2. Cancel (discard)\n\n";

        int finishChoice = readInt("Choice: ");
        if (finishChoice != 1) {
            std::cout << "Workout discarded.\n";
            pause();
            return;
        }

        auto endedAt = std::chrono::steady_clock::now();
        session.durationSeconds = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(endedAt - startedAt).count()
        );

        try {
            workoutSvc.endWorkoutSession(session);
            std::cout << "Saved! Session ID: " << session.sessionId << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error saving workout: " << e.what() << "\n";
        }
        pause();
        return;
    }
}

void showHistoryScreen(
    int userId,
    const std::string& username,
    WorkoutSessionService& workoutSvc
) {
    while (true) {
        clearScreen();
        std::cout << "=== History ===\n";
        std::cout << "Logged in as: " << username << "\n\n";

        auto sessions = workoutSvc.getWorkoutHistory(userId);
        if (sessions.empty()) {
            std::cout << "No workouts logged yet.\n";
            pause();
            return;
        }

        std::cout << "Your sessions (newest first):\n";
        for (size_t i = 0; i < sessions.size(); ++i) {
            const auto& s = sessions[i];
            std::cout << "  " << (i + 1) << ". " << s.performedAt
                      << " | " << (s.sourceLabel.empty() ? "workout" : s.sourceLabel)
                      << " | " << s.durationSeconds << "s"
                      << " | id=" << s.sessionId << "\n";
        }
        std::cout << "\n0. Back\n";

        int idx = readInt("Open session #: ");
        if (idx == 0) return;
        if (idx < 1 || idx > static_cast<int>(sessions.size())) continue;

        WorkoutSession selected = sessions[idx - 1];
        try {
            workoutSvc.loadWorkoutSessionDetails(selected);
        } catch (const std::exception& e) {
            std::cout << "Error loading session details: " << e.what() << "\n";
            pause();
            continue;
        }

        clearScreen();
        std::cout << "=== Session Details ===\n\n";
        std::cout << "Performed at: " << selected.performedAt << "\n";
        std::cout << "Source: " << (selected.sourceLabel.empty() ? "workout" : selected.sourceLabel) << "\n";
        std::cout << "Duration: " << selected.durationSeconds << " seconds\n";
        std::cout << "Total volume: " << selected.calculateTotalVolume() << " kg\n\n";

        if (selected.exercises.empty()) {
            std::cout << "No exercise data found for this session.\n";
        } else {
            for (const auto& ex : selected.exercises) {
                std::cout << "- " << ex.exerciseNameSnapshot << "\n";
                if (ex.sets.empty()) {
                    std::cout << "  (No sets)\n";
                } else {
                    for (size_t i = 0; i < ex.sets.size(); ++i) {
                        std::cout << "  Set " << (i + 1) << ": "
                                  << ex.sets[i].reps << " reps @ "
                                  << ex.sets[i].weight << " kg\n";
                    }
                }
                std::cout << "\n";
            }
        }

        pause();
    }
}

/**
 * @brief Shows aggregate analytics for a wide default date range.
 * @author Dhruvil Patel
 * @param userId Active user id.
 * @param analyticsSvc Service that loads dashboard totals.
 */
void showAnalyticsScreen(int userId, AnalyticsDashboardService& analyticsSvc) {
    clearScreen();
    std::cout << "=== Analytics ===\n\n";
    try {
        auto dashboard = analyticsSvc.getDashboardCustom(userId, "2020-01-01", "2030-12-31");
        if (!dashboard.hasWorkouts) {
            std::cout << "No workouts logged yet.\n";
            std::cout << "Analytics will appear after you log workouts.\n";
        } else {
            std::cout << "Workouts: " << dashboard.workoutCount << "\n";
            std::cout << "Total sets: " << dashboard.totalSets << "\n";
            std::cout << "Total reps: " << dashboard.totalReps << "\n";
            std::cout << "Total volume: " << dashboard.totalVolume << " kg\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error loading analytics: " << e.what() << "\n";
    }
    pause();
}

/**
 * @brief Planner UI: schedule template days, view today / week, or clear planned rows.
 * @details Delegates persistence to @c Scheduler which uses the @c planned_workouts table.
 * @author Dhruvil Patel
 * @param scheduler Database-backed scheduler for the current user.
 * @param daySvc Used to resolve template day titles when scheduling.
 */
void showPlannerScreen(Scheduler& scheduler, TemplateDayService& daySvc) {
    while (true) {
        clearScreen();
        std::cout << "=== Planner ===\n\n";
        std::cout << "1. Schedule a Template\n";
        std::cout << "2. View Today's Workouts\n";
        std::cout << "3. View Weekly Overview\n";
        std::cout << "4. Clear Schedule\n";
        std::cout << "5. Back to menu\n\n";

        int choice = readInt("Choice: ");
        if (choice == 1) {
            int templateId = readInt("Enter Template Day ID to schedule: ");

            std::cout << "\nWhen would you like to schedule this for?\n";
            std::cout << "  1. Today\n";
            std::cout << "  2. Custom Date\n";
            int dateChoice = readInt("Choice: ");

            DateTime dt = DateTime::now();
            if (dateChoice == 2) {
                dt.year = readInt("Year (e.g. 2026): ");
                dt.month = readInt("Month (1-12): ");
                dt.day = readInt("Day (1-31): ");
            }

            scheduler.assignTemplateToDay(daySvc, templateId, dt);
            std::cout << "Workout scheduled and saved to database!\n";
            pause();
        } else if (choice == 2) {
            auto todaysWorkouts = scheduler.getTodaysWorkout(DateTime::now());
            if (todaysWorkouts.empty()) {
                std::cout << "\nNo workouts scheduled for today.\n";
            } else {
                std::cout << "\nWorkouts for today:\n";
                for (const auto& sw : todaysWorkouts) {
                    std::cout << "  * [" << sw.status << "] " << sw.titleSnapshot
                              << " (Template ID: " << sw.templateDayId << ")\n";
                }
            }
            pause();
        } else if (choice == 3) {
            auto weekWorkouts = scheduler.getWeeklyOverview(DateTime::now());
            if (weekWorkouts.empty()) {
                std::cout << "\nNo workouts scheduled for the next 7 days.\n";
            } else {
                std::cout << "\nWeekly Overview (Next 7 Days):\n";
                for (const auto& sw : weekWorkouts) {
                    std::cout << "  * " << sw.plannedFor << " | [" << sw.status << "] " << sw.titleSnapshot
                              << " (Template ID: " << sw.templateDayId << ")\n";
                }
            }
            pause();
        } else if (choice == 4) {
            scheduler.clearSchedule();
            std::cout << "Schedule cleared from database.\n";
            pause();
        } else if (choice == 5) {
            return;
        }
    }
}

/**
 * @brief Goals UI: view streaks, update streak, set frequency goals, list active goals.
 * @author Dhruvil Patel
 * @param tracker Streak state loaded from the @c streaks table.
 * @param goalManager Persists and reads rows in the @c goals table.
 */
void showGoalsScreen(StreakTracker& tracker, GoalManager& goalManager) {
    while (true) {
        clearScreen();
        std::cout << "=== Goals & Streaks ===\n\n";
        std::cout << "Current Streak: " << tracker.getCurrentStreak() << " days\n";
        std::cout << "Best Streak: " << tracker.getBestStreak() << " days\n";
        std::cout << "Last Workout: " << (tracker.getLastWorkoutDate().empty() ? "None" : tracker.getLastWorkoutDate()) << "\n\n";

        std::cout << "1. Log Workout (Updates Database Streak)\n";
        std::cout << "2. Set Weekly Frequency Goal\n";
        std::cout << "3. View Active Goals\n";
        std::cout << "4. Back to menu\n\n";

        int choice = readInt("Choice: ");
        if (choice == 1) {
            tracker.updateStreak();
            std::cout << "Streak updated and saved!\n";
            pause();
        } else if (choice == 2) {
            int freq = readInt("Target workouts per week: ");
            if (freq > 0) {
                goalManager.setFrequencyGoal(freq);
                std::cout << "Goal saved to database!\n";
            }
            pause();
        } else if (choice == 3) {
            std::cout << "\n--- Active Goals ---\n";
            auto goals = goalManager.getActiveGoals();
            if (goals.empty()) std::cout << "No active goals.\n";
            for (const auto& g : goals) std::cout << "  * " << g << "\n";
            pause();
        } else if (choice == 4) {
            return;
        }
    }
}

/**
 * @brief Main application loop after the database is opened: login then menu dispatch.
 * @author Dhruvil Patel
 * @param db Open SQLite database handle.
 * @param userSvc Account login and registration.
 * @param progSvc Program CRUD.
 * @param daySvc Template days for programs and planner validation.
 * @param analyticsSvc Dashboard aggregates for the analytics screen.
 */
void runApp(Database& db, UserAccountService& userSvc, ProgramService& progSvc,
            TemplateDayService& daySvc, ExerciseRepository& exRepo,
            TemplateExerciseService& templateExerciseSvc,
            WorkoutSessionService& workoutSvc, AnalyticsDashboardService& analyticsSvc) {
    int currentUserId = 0;
    std::string currentUsername;

    showLoginScreen(userSvc, currentUserId, currentUsername);

    Section current = Section::Dashboard;
    while (true) {
        clearScreen();
        std::cout << "=== VolumeLab ===\n";
        std::cout << "Logged in as: " << currentUsername << "\n";
        showMainMenu(current);

        int choice = readInt("Choice: ");
        if (choice == 0) {
            std::cout << "Logged out.\n";
            pause();
            showLoginScreen(userSvc, currentUserId, currentUsername);
        }
        if (choice >= 1 && choice <= 7) {
            current = static_cast<Section>(choice - 1);
        }

        Scheduler scheduler(db, currentUserId);
        StreakTracker streakTracker(db, currentUserId);
        GoalManager goalManager(db, currentUserId, &streakTracker);

        switch (current) {
        case Section::Dashboard:
            showDashboard(currentUserId, currentUsername, progSvc);
            break;
        case Section::Programs:
            showProgramsScreen(currentUserId, progSvc, daySvc, exRepo, templateExerciseSvc);
            continue;
        case Section::Workout:
            showWorkoutScreen(currentUserId, currentUsername, progSvc, daySvc, exRepo, templateExerciseSvc, workoutSvc);
            continue;
        case Section::History:
            showHistoryScreen(currentUserId, currentUsername, workoutSvc);
            continue;
        case Section::Analytics:
            showAnalyticsScreen(currentUserId, analyticsSvc);
            continue;
        case Section::Planner:
            showPlannerScreen(scheduler, daySvc);
            break;
        case Section::Goals:
            showGoalsScreen(streakTracker, goalManager);
            break;
        }
        pause();
    }
}

/**
 * @brief Program entry: opens the database, constructs services, and runs @ref runApp.
 * @author Dhruvil Patel
 * @return 0 on normal exit, 1 if an exception escapes the UI loop.
 */
int main() {
    const std::string dbPath = "data/volumelab.db";
    Database db(dbPath);
    db.open();

    UserRepository userRepo(db);
    UserAccountService userSvc(userRepo);

    ProgramRepository progRepo(db);
    ProgramService progSvc(progRepo);

    TemplateDayRepository dayRepo(db);
    TemplateDayService daySvc(dayRepo);

    TemplateExerciseRepository templateExerciseRepo(db);
    TemplateExerciseService templateExerciseSvc(templateExerciseRepo);

    ExerciseRepository exRepo(db);

    WorkoutSessionRepository workoutRepo(db);
    WorkoutSessionService workoutSvc(workoutRepo);

    WeeklyTrendsRepository trendsRepo(db);
    AnalyticsDashboardRepository analyticsRepo(db);
    AnalyticsDashboardService analyticsSvc(analyticsRepo, trendsRepo, db);

    try {
        runApp(db, userSvc, progSvc, daySvc, exRepo, templateExerciseSvc, workoutSvc, analyticsSvc);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
