#ifndef VIDEOSCREENSHOTWIDGET_H
#define VIDEOSCREENSHOTWIDGET_H

#include <QWidget>

namespace Ui {
class VideoScreenshotWidget;
}

/**
 * @brief subclass of QWidget that scales the screenshot pixmap, keeping the aspect ratio
 * of the image.
 */
class VideoScreenshotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoScreenshotWidget(QWidget *parent = 0);
    ~VideoScreenshotWidget();

public slots:
    /** Set a new pixmap. This will repaint the widget */
    void setPixmap(const QPixmap &);

protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    QPixmap _videoScreenshot;
    Ui::VideoScreenshotWidget *ui;
};

#endif // VIDEOSCREENSHOTWIDGET_H
