#pragma once
#include <string>
#include <vector>

struct PresetExercise {
    std::string name;
    int sets;
    std::string reps;
    double weightKg;
};

struct PresetTemplateDay {
    std::string name;
    std::vector<PresetExercise> exercises;
};

struct PresetProgram {
    std::string name;
    std::vector<PresetTemplateDay> days;
};

// Full preset programs
PresetProgram makeUpperLowerProgram();
PresetProgram makePushPullLegsProgram();
PresetProgram makeFullBodyProgram();

std::vector<PresetProgram> getPresetPrograms();

// Standalone preset template days
PresetTemplateDay makeUpperBodyDay();
PresetTemplateDay makeLowerBodyDay();
PresetTemplateDay makePushDay();
PresetTemplateDay makePullDay();
PresetTemplateDay makeLegDay();
PresetTemplateDay makeChestTricepsDay();
PresetTemplateDay makeBackBicepsDay();
PresetTemplateDay makeShouldersDay();
PresetTemplateDay makeArmsDay();
PresetTemplateDay makeFullBodyDay();

std::vector<PresetTemplateDay> getPresetTemplateDays();
