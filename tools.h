#ifndef TOOLS_H
#define TOOLS_H
#include "mainwindow.h"
#include <QtCharts/QChartView>

class Tools
{
public:
    Tools();
    void greyscale(Image &image);
    void quantize(Image &image);
    void verticalMirror(Image &image);
    void horizontalMirror(Image &image);
    QChartView* lumHistogram(Image &image);

private:
    QChartView *histogramView = nullptr;
    std::array<unsigned int, 256> hist = {0};
};

#endif // TOOLS_H
