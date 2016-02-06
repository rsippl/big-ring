#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <functional>
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

    void withMutex(const std::function<void(void *ptr, const QSize &frameSize, int index)> func) {
        QMutexLocker locker(&_mutex);

        func(_ptr, _frameSize, _index);
    }

    void reset() {
        QMutexLocker locker(&_mutex);
        _ptr = nullptr;
    }

private:
    void* _ptr;
    const QSize _frameSize;
    const int _index;
    QMutex _mutex;
};

#endif // FRAMEBUFFER_H
