#pragma once

typedef enum GraphType
{
    GRAPH_Z_RE,
    GRAPH_Z_IM,
    GRAPH_Z_RE_Z_IM,
    GRAPH_Z_PHASE,
} GraphType;

typedef enum
{
    VM_LC,
    VM_Z_REAL_IMAG,
    VM_Z_ABS_ARG,
} VIEW_MODE;

typedef struct Settings
{
    bool view_parallel;
    VIEW_MODE view_mode;

    //Индекс, для StandartFreq функции
    int single_freq_index;

    int32_t graph_min_freq;
    int32_t graph_max_freq;
    GraphType graph_type;
} Settings;

extern Settings g_settings;

void LoadSettings();
bool SaveSettingsIfChangedAndTimeUp();
