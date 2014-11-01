#ifndef PREVIEWVIDEOWIDGET_H
#define PREVIEWVIDEOWIDGET_H

#include <QtCore/QtGlobal>
#include <QGst/Ui/VideoWidget>
#include <QGst/Message>
#include <QGst/Pipeline>


class PreviewVideoWidget: public QGst::Ui::VideoWidget
{
    Q_OBJECT
public:
    PreviewVideoWidget(QWidget* parent);
    virtual ~PreviewVideoWidget();
    void setUri(QString uri);
signals:
    void stateChanged();

public slots:
    void play();
    void step();
private:
    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);
    void stop();

    QGst::PipelinePtr _pipeline;
    QTimer* _stepTimer;
};

#endif // PREVIEWVIDEOWIDGET_H
