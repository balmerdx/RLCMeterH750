#pragma once

typedef enum GraphType
{
    GRAPH_Z_RE,
    GRAPH_Z_IM,
    GRAPH_Z_RE_Z_IM,
    GRAPH_Z_PHASE,
} GraphType;

extern int g_min_freq;
extern int g_max_freq;

extern GraphType g_graph_type;

void SceneGraphStart();

//Запусткает сканирование интервала.
void SceneGraphStartScan();

int printEvenFreq(char* str, int freq, int length);
void SceneGraphResultZx();
