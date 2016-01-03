#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QtCore/QSize>

struct FrameBuffer
{
    void* ptr;
    QSize frameSize;
    int index;
};

#endif // FRAMEBUFFER_H
