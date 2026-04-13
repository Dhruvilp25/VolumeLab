# VolumeLab Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wall
LDFLAGS = -lsqlite3

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += -D__APPLE__
endif
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lcrypto
endif

SRC = src/database.cpp \
      src/user_repository.cpp \
      src/user_account_service.cpp \
      src/password_util.cpp \
      src/program.cpp \
      src/template_day.cpp \
      src/template_exercise.cpp \
      src/exercise_repository.cpp \
      src/weekly_trends_repository.cpp \
      src/analytics_dashboard_repository.cpp \
      src/analytics_dashboard_service.cpp \
      src/volume_wrapped_repository.cpp \
      src/volume_wrapped_service.cpp \
	  src/workout_session.cpp \
	  src/workout_session_repository.cpp \
	  src/workout_session_service.cpp\
	  src/scheduler.cpp \
	  src/streaktracker.cpp \
      src/graph_renderer.cpp \
      src/plateau_regression_repository.cpp \
      src/plateau_regression_service.cpp


.PHONY: all run init seed-demo test clean

all: volumelab

volumelab: $(SRC) src/main.cpp
	$(CXX) $(CXXFLAGS) $(SRC) src/main.cpp -o volumelab $(LDFLAGS)

run: volumelab init
	./volumelab

init:
	@mkdir -p data
	@if [ ! -f data/volumelab.db ]; then \
		./db/init_db.sh; \
		echo "Database initialized."; \
	fi

seed-demo: init
	sqlite3 -bail data/volumelab.db < db/seed_demo.sql
	@echo "Demo data seeded."
	@echo "Login with username: demo / password: demo123"

# Run all acceptance tests
test: test_weekly_trends test_analytics_dashboard test_volume_wrapped test_user_accounts test_programs test_workout_sessions test_scheduler test_streaktracker test_graph_renderer test_plateau_regression

test_weekly_trends:
	$(CXX) $(CXXFLAGS) src/database.cpp src/weekly_trends_repository.cpp tests/test_weekly_trends.cpp -o test_weekly_trends $(LDFLAGS)
	./test_weekly_trends

test_analytics_dashboard:
	$(CXX) $(CXXFLAGS) src/database.cpp src/weekly_trends_repository.cpp src/analytics_dashboard_repository.cpp src/analytics_dashboard_service.cpp tests/test_analytics_dashboard.cpp -o test_analytics_dashboard $(LDFLAGS)
	./test_analytics_dashboard

test_volume_wrapped:
	$(CXX) $(CXXFLAGS) src/database.cpp src/weekly_trends_repository.cpp src/analytics_dashboard_repository.cpp src/analytics_dashboard_service.cpp src/volume_wrapped_repository.cpp src/volume_wrapped_service.cpp tests/test_volume_wrapped.cpp -o test_volume_wrapped $(LDFLAGS)
	./test_volume_wrapped

test_user_accounts:
	$(CXX) $(CXXFLAGS) src/database.cpp src/user_repository.cpp src/user_account_service.cpp src/password_util.cpp tests/test_user_accounts.cpp -o test_user_accounts $(LDFLAGS)
	./test_user_accounts

test_programs:
	$(CXX) $(CXXFLAGS) src/database.cpp src/program.cpp tests/test_programs.cpp -o test_programs $(LDFLAGS)
	./test_programs
test_workout_sessions:
	$(CXX) $(CXXFLAGS) src/database.cpp src/workout_session.cpp tests/test_workout_sessions.cpp -o test_workout_sessions $(LDFLAGS)
	./test_workout_sessions
test_streaktracker:
	$(CXX) $(CXXFLAGS) src/database.cpp src/streaktracker.cpp tests/test_streaktracker.cpp -o test_streaktracker $(LDFLAGS)
	./test_streaktracker
test_scheduler:
	$(CXX) $(CXXFLAGS) src/database.cpp src/scheduler.cpp tests/test_scheduler.cpp -o test_scheduler $(LDFLAGS)
	./test_scheduler
test_graph_renderer:
	$(CXX) $(CXXFLAGS) src/graph_renderer.cpp tests/test_graph_renderer.cpp -o test_graph_renderer
	./test_graph_renderer

test_plateau_regression:
	$(CXX) $(CXXFLAGS) src/database.cpp src/plateau_regression_repository.cpp src/plateau_regression_service.cpp tests/test_plateau_regression.cpp -o test_plateau_regression $(LDFLAGS)
	./test_plateau_regression

clean:
	rm -f volumelab test_weekly_trends test_analytics_dashboard test_volume_wrapped test_user_accounts test_programs test_workout_sessions test_streaktracker test_scheduler test_graph_renderer test_plateau_regression