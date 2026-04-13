# VolumeLab

**VolumeLab** is a C++ and SQLite-based fitness tracking application designed to help users build workout programs, log training sessions, monitor progress, and analyze long-term workout trends. The project focuses on structured workout data, performance tracking, and analytics, with support for program templates, workout history, scheduling, and progress insights.

## Project Overview

VolumeLab is a workout tracking and analytics system built to give users a structured way to manage their training. Core capabilities include:

- Creating and managing workout programs
- Logging completed workout sessions
- Tracking sets, reps, weights, and exercise volume
- Viewing workout history over time
- Analyzing weekly training trends
- Supporting future features such as scheduling, streak tracking, goals, and summary analytics

## Tech Stack

- **Language:** C++
- **Database:** SQLite
- **Build Tool:** Make
- **Version Control:** GitLab

## Prerequisites

Before building or testing the project, ensure the following are installed:

- `g++` with C++17 support
- `make`
- `sqlite3`
- SQLite development library

Example setup:

**macOS**
```bash
brew install sqlite
g++ --version
sqlite3 --version
make --version
````

**Windows**
Install the following first:

* MinGW-w64 or another `g++` compiler with C++17 support
* `make`
* SQLite Tools from the official SQLite website
* SQLite development files if they are not already included with your compiler setup

## Build and Run the Application

Run all commands from the repository root.

Build the application:

```bash
make
```

Initialize the database if needed:

```bash
make init
```

Run the application:

```bash
./volumelab
```

Or run everything in one step:

```bash
make run
```

### Seed Showcase Demo Data

If you want pre-populated data for a live demo (programs, template days, exercises, and workout history):

```bash
make seed-demo
```

Then run:

```bash
./volumelab
```

Demo credentials:

- Username: `demo`
- Password: `demo123`

## Stage 3 Acceptance Test Instructions

Run all commands from the repository root.

To compile and run **all Stage 3 acceptance tests**, use:

```bash
make test
```

This command runs the following test targets:

* `test_weekly_trends`
* `test_analytics_dashboard`
* `test_volume_wrapped`
* `test_user_accounts`
* `test_programs`
* `test_workout_sessions`
* `test_scheduler`
* `test_streaktracker`

If all tests pass, the terminal will print the corresponding `PASS: ...` message for each test.

Example successful output will include messages like:

```bash
PASS: Weekly volume & frequency trends acceptance tests
PASS: Analytics dashboard acceptance tests
PASS: Volume Wrapped acceptance tests
PASS: User account acceptance tests
PASS: Programs acceptance tests
PASS: Workout session acceptance tests
PASS: StreakTracker tests
PASS: Scheduler tests
```

### Running Individual Acceptance Tests

If needed, each acceptance test can also be run separately:

```bash
make test_weekly_trends
make test_analytics_dashboard
make test_volume_wrapped
make test_user_accounts
make test_programs
make test_workout_sessions
make test_scheduler
make test_streaktracker
```

### Notes for the TA

* No manual database setup is required before running the acceptance tests.
* The acceptance tests create and use their own temporary SQLite databases.
* The schema is loaded automatically by the tests where needed.
* For grading Stage 3, `make test` is the intended command.

## Local Database Setup

This section is only needed for running the application locally, not for acceptance test grading.

### What's Included

* `db/schema.sql` — defines the database tables, constraints, and indexes
* `db/init_db.sh` — initializes a fresh local SQLite database from the schema
* `data/` — local-only directory where `volumelab.db` is created

### Create the Database Locally

```bash
./db/init_db.sh
```

This creates the local database file used by the application.

## Clean Build Files

To remove compiled executables:

```bash
make clean
```
