#pragma once
/*
 * Построение графиков.
 */

typedef struct Point
{
    float x;
    float y;
} Point;

#define PLOT_MAX_POINTS 256
#define PLOT_MAX_GRAPH 2


//Размеры области, занимаемой графиком.
void PlotInit(int x, int y, int dx, int dy);

void PlotSetAxis(float xmin, float xmax, float ymin, float ymax);

void PlotDrawGraph(int graph_index, Point* points, int count, uint16_t colorW);

//Горизонтальная линия, которой можно задавать положение
void PlotLineSetVisible(bool show);
bool PlotLineVisible();

//x = положение линии в пикселях
//0 - начало графика
//dx из PlotInit - конец графика
void PlotLineSetPos(int x);

//x - значения от xmin до xmax в PlotSetAxis
void PlotLineSetPosX(float x);

// возвращает положение линии в пикселях
int PlotLinePos();
// возвращает положение линии координатах графика
float PlotLineX();
uint64_t PlotLineXround();
