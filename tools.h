#ifndef TOOLS_H
#define TOOLS_H
#include "mainwindow.h"

class Tools
{
public:
    Tools();
    void greyscale(Image &image);
    void quantize(Image &image);
    void verticalMirror(Image &image);
    void horizontalMirror(Image &image);
};

#endif // TOOLS_H
