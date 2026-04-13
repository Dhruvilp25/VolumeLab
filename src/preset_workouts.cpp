#include "preset_workouts.hpp"

// Standalone Preset Template Day

PresetTemplateDay makeUpperBodyDay() {
    return {"Upper Body", {
        {"Bench Press",            4, "6-8",   0},
        {"Incline Dumbbell Press", 3, "8-10",  0},
        {"Lat Pulldown",           3, "8-12",  0},
        {"Seated Cable Row",       3, "8-12",  0},
        {"Dumbbell Shoulder Press", 3, "8-10", 0},
        {"Lateral Raise",          3, "12-15", 0},
        {"Barbell Curl",           3, "10-12", 0},
        {"Tricep Pushdown",        3, "10-12", 0},
    }};
}

PresetTemplateDay makeLowerBodyDay() {
    return {"Lower Body", {
        {"Back Squat",       4, "5-8",   0},
        {"Romanian Deadlift", 3, "8-10", 0},
        {"Leg Press",        3, "10-12", 0},
        {"Leg Curl",         3, "10-12", 0},
        {"Leg Extension",    3, "12-15", 0},
        {"Calf Raise",       4, "12-15", 0},
    }};
}

PresetTemplateDay makePushDay() {
    return {"Push Day", {
        {"Bench Press",                4, "6-8",   0},
        {"Incline Bench Press",        3, "8-10",  0},
        {"Overhead Press",             3, "6-8",   0},
        {"Lateral Raise",              3, "12-15", 0},
        {"Tricep Pushdown",            3, "10-12", 0},
        {"Overhead Tricep Extension",  3, "10-12", 0},
    }};
}

PresetTemplateDay makePullDay() {
    return {"Pull Day", {
        {"Deadlift",                    3, "3-5",   0},
        {"Pull Up / Assisted Pull Up",  3, "6-10",  0},
        {"Barbell Row",                 3, "6-8",   0},
        {"Seated Cable Row",            3, "8-12",  0},
        {"Face Pull",                   3, "12-15", 0},
        {"Barbell Curl",                3, "10-12", 0},
        {"Hammer Curl",                 3, "10-12", 0},
    }};
}

PresetTemplateDay makeLegDay() {
    return {"Leg Day", {
        {"Back Squat",       4, "5-8",    0},
        {"Romanian Deadlift", 3, "8-10",  0},
        {"Walking Lunges",   3, "10/leg", 0},
        {"Leg Press",        3, "10-12",  0},
        {"Hamstring Curl",   3, "10-12",  0},
        {"Calf Raise",       4, "12-15",  0},
    }};
}

PresetTemplateDay makeChestTricepsDay() {
    return {"Chest + Triceps", {
        {"Bench Press",              4, "6-8",   0},
        {"Incline Dumbbell Press",   3, "8-10",  0},
        {"Cable Fly",                3, "12-15", 0},
        {"Dips / Assisted Dips",     3, "8-12",  0},
        {"Tricep Pushdown",          3, "10-12", 0},
        {"Skull Crushers",           3, "10-12", 0},
    }};
}

PresetTemplateDay makeBackBicepsDay() {
    return {"Back + Biceps", {
        {"Pull Up / Assisted Pull Up", 3, "6-10",  0},
        {"Barbell Row",                4, "6-8",   0},
        {"Lat Pulldown",               3, "8-12",  0},
        {"Seated Cable Row",           3, "8-12",  0},
        {"Barbell Curl",               3, "10-12", 0},
        {"Incline Dumbbell Curl",      3, "10-12", 0},
    }};
}

PresetTemplateDay makeShouldersDay() {
    return {"Shoulders", {
        {"Overhead Press",          4, "6-8",   0},
        {"Dumbbell Shoulder Press", 3, "8-10",  0},
        {"Lateral Raise",           4, "12-15", 0},
        {"Rear Delt Fly",           3, "12-15", 0},
        {"Upright Row",             3, "10-12", 0},
        {"Shrugs",                  3, "10-12", 0},
    }};
}

PresetTemplateDay makeArmsDay() {
    return {"Arms", {
        {"Barbell Curl",               3, "10-12", 0},
        {"Hammer Curl",                3, "10-12", 0},
        {"Preacher Curl",              3, "10-12", 0},
        {"Tricep Pushdown",            3, "10-12", 0},
        {"Skull Crushers",             3, "10-12", 0},
        {"Overhead Tricep Extension",  3, "10-12", 0},
    }};
}

PresetTemplateDay makeFullBodyDay() {
    return {"Full Body", {
        {"Back Squat",              3, "5-8",   0},
        {"Bench Press",             3, "6-8",   0},
        {"Barbell Row",             3, "6-8",   0},
        {"Romanian Deadlift",       3, "8-10",  0},
        {"Dumbbell Shoulder Press", 2, "8-10",  0},
        {"Barbell Curl",            2, "10-12", 0},
        {"Tricep Pushdown",         2, "10-12", 0},
    }};
}

std::vector<PresetTemplateDay> getPresetTemplateDays() {
    return {
        makeUpperBodyDay(),
        makeLowerBodyDay(),
        makePushDay(),
        makePullDay(),
        makeLegDay(),
        makeChestTricepsDay(),
        makeBackBicepsDay(),
        makeShouldersDay(),
        makeArmsDay(),
        makeFullBodyDay(),
    };
}

// ── Full Preset Programs ─────────────────────────────────────────

PresetProgram makeUpperLowerProgram() {
    return {"Upper / Lower 4-Day", {
        {"Upper A", makeUpperBodyDay().exercises},
        {"Lower A", makeLowerBodyDay().exercises},
        {"Upper B", {
            {"Overhead Press",              4, "6-8",   0},
            {"Dumbbell Shoulder Press",     3, "8-10",  0},
            {"Pull Up / Assisted Pull Up",  3, "6-10",  0},
            {"Lat Pulldown",                3, "8-12",  0},
            {"Incline Dumbbell Press",      3, "8-10",  0},
            {"Face Pull",                   3, "12-15", 0},
            {"Hammer Curl",                 3, "10-12", 0},
            {"Overhead Tricep Extension",   3, "10-12", 0},
        }},
        {"Lower B", {
            {"Deadlift",          3, "3-5",   0},
            {"Leg Press",         4, "10-12", 0},
            {"Walking Lunges",    3, "10/leg", 0},
            {"Hamstring Curl",    3, "10-12", 0},
            {"Leg Extension",     3, "12-15", 0},
            {"Calf Raise",        4, "12-15", 0},
        }},
    }};
}

PresetProgram makePushPullLegsProgram() {
    return {"Push Pull Legs", {
        makePushDay(),
        makePullDay(),
        makeLegDay(),
    }};
}

PresetProgram makeFullBodyProgram() {
    return {"Full Body 3-Day", {
        {"Full Body A", {
            {"Back Squat",              4, "5-8",   0},
            {"Bench Press",             3, "6-8",   0},
            {"Barbell Row",             3, "6-8",   0},
            {"Romanian Deadlift",       3, "8-10",  0},
            {"Dumbbell Shoulder Press", 2, "8-10",  0},
            {"Barbell Curl",            2, "10-12", 0},
            {"Tricep Pushdown",         2, "10-12", 0},
        }},
        {"Full Body B", {
            {"Deadlift",                3, "3-5",   0},
            {"Incline Dumbbell Press",  3, "8-10",  0},
            {"Lat Pulldown",            3, "8-12",  0},
            {"Overhead Press",          3, "6-8",   0},
            {"Leg Curl",                3, "10-12", 0},
            {"Hammer Curl",             2, "10-12", 0},
            {"Skull Crushers",          2, "10-12", 0},
        }},
        {"Full Body C", {
            {"Leg Press",               4, "10-12", 0},
            {"Bench Press",             3, "6-8",   0},
            {"Seated Cable Row",        3, "8-12",  0},
            {"Walking Lunges",          3, "10/leg", 0},
            {"Lateral Raise",           3, "12-15", 0},
            {"Barbell Curl",            2, "10-12", 0},
            {"Overhead Tricep Extension", 2, "10-12", 0},
        }},
    }};
}

std::vector<PresetProgram> getPresetPrograms() {
    return {
        makeUpperLowerProgram(),
        makePushPullLegsProgram(),
        makeFullBodyProgram(),
    };
}
