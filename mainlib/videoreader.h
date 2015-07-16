#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include "genericvideoreader.h"

#include <QtCore/QEvent>
#include <QtCore/QObject>

class RealLifeVideo;

struct SwsContext;

class VideoReader : public GenericVideoReader
{
    Q_OBJECT
public:
    explicit VideoReader(QObject *parent = 0);
    virtual ~VideoReader();

    void createImageForFrame(const RealLifeVideo& rlv, const qreal distance);
signals:
    void error(const QString& errorMessage);
    void videoOpened(const QString& videoFilename, const QSize& videoSize);
    void newFrameReady(const RealLifeVideo& rlv, qreal distance, const QImage& frame);
    void seekReady(qint64 frameNumber);

protected:
    virtual bool event(QEvent *);
private:
    void initialize();
    virtual void openVideoFileInternal(const QString& videoFilename) override;

    void createImageForFrameNumber(RealLifeVideo &rlv, const qreal distance);
    QImage createImage();

    bool _initialized;
    // libav specific data
    QScopedPointer<AVFrameWrapper> _frameRgb;
    SwsContext* _swsContext;
    QByteArray _imageBuffer;
};

#endif // VIDEOREADER_H
