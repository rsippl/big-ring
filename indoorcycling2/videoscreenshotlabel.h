#ifndef VIDEOSCREENSHOTLABEL_H
#define VIDEOSCREENSHOTLABEL_H

#include <QtWidgets/QLabel>

/**
 * @brief subclass of QLabel that scales the screenshot pixmap, keeping the aspect ratio
 * of the image.
 */
class VideoScreenshotLabel : public QLabel
{
    Q_OBJECT
public:
    explicit VideoScreenshotLabel(QWidget *parent = 0);

    void setPixmap(const QPixmap &);

    virtual void resizeEvent(QResizeEvent *) override;
private:
    void scaleAndPlacePixmap();
    QPixmap _videoScreenshot;
};

#endif // VIDEOSCREENSHOTLABEL_H
