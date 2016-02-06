#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QtCore/QMutex>
#include <QtCore/QSize>

class FrameBuffer
{
public:
    FrameBuffer(void *mappedPixelBuffer, const QSize &frameSize, int index):
        _ptr(mappedPixelBuffer), _frameSize(frameSize), _index(index)
    {
        // empty
    }

    ~FrameBuffer() {
        // no need to do anything
    }

    void *mappedBufferPointer() const {
        return _ptr;
    }

    void reset() {
        _ptr = nullptr;
    }

    QMutex &mutex() {
        return _mutex;
    }

    QSize frameSize() const {
        return _frameSize;
    }

    int index() const {
        return _index;
    }
private:
    void* _ptr;
    const QSize _frameSize;
    const int _index;
    QMutex _mutex;
};

#endif // FRAMEBUFFER_H
