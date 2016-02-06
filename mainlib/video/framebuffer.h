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

    void withMutex(const std::function<void(FrameBuffer&)> func) {
        QMutexLocker locker(&_mutex);
        func(*this);
    }

    void *mappedBufferPointer() const {
        return _ptr;
    }

    void reset() {
        _ptr = nullptr;
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
