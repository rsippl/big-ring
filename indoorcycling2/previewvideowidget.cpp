#include "previewvideowidget.h"
#include <QTimer>
#include <QUrl>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/Bus>
#include <QGst/Event>
#include <QGlib/Connect>

PreviewVideoWidget::PreviewVideoWidget(QWidget* parent):
    QGst::Ui::VideoWidget(parent), _stepTimer(new QTimer(this))
{
    _stepTimer->setInterval(1000 / 30);
    connect(_stepTimer, &QTimer::timeout, this, &PreviewVideoWidget::step);

}

PreviewVideoWidget::~PreviewVideoWidget()
{
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        stopPipelineWatch();
    }
}

void PreviewVideoWidget::setUri(QString uri)
{
    stop();
    //if uri is not a real uri, assume it is a file path
    if (uri.indexOf("://") < 0) {
        uri = QUrl::fromLocalFile(uri).toEncoded();
    }

    if (!_pipeline) {
        _pipeline = QGst::ElementFactory::make("playbin").dynamicCast<QGst::Pipeline>();
        if (_pipeline) {
            //let the video widget watch the pipeline for new video sinks
            watchPipeline(_pipeline);

            //watch the bus for messages
            QGst::BusPtr bus = _pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &PreviewVideoWidget::onBusMessage);
        } else {
            qCritical() << "Failed to create the pipeline";
        }
    }

    if (_pipeline) {
        _pipeline->setProperty("uri", uri);
    }

}

void PreviewVideoWidget::play()
{
    if (_pipeline) {
        _pipeline->setState(QGst::StatePaused);
        _stepTimer->start();
    }
}

void PreviewVideoWidget::step()
{
    QGst::EventPtr stepEvent = QGst::StepEvent::create(QGst::FormatBuffers, 1, 1.0, true, false);
    _pipeline->sendEvent(stepEvent);
}

void PreviewVideoWidget::stop()
{
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _stepTimer->stop();

        //once the pipeline stops, the bus is flushed so we will
        //not receive any StateChangedMessage about this.
        //so, to inform the ui, we have to emit this signal manually.
        Q_EMIT stateChanged();
    }
}


void PreviewVideoWidget::onBusMessage(const QGst::MessagePtr &message)
{
    switch (message->type()) {
    case QGst::MessageEos: //End of stream. We reached the end of the file.
        stop();
        break;
    case QGst::MessageError: //Some error occurred.
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        stop();
        break;
    case QGst::MessageStateChanged: //The element in message->source() has changed state
        if (message->source() == _pipeline) {
            handlePipelineStateChange(message.staticCast<QGst::StateChangedMessage>());
        }
        break;
    default:
        break;
    }
}

void PreviewVideoWidget::handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm)
{
    switch (scm->newState()) {
    case QGst::StatePlaying:
        //start the timer when the pipeline starts playing
//        m_positionTimer.start(100);
        break;
    case QGst::StatePaused:
        //stop the timer when the pipeline pauses
        if(scm->oldState() == QGst::StatePlaying) {
//            m_positionTimer.stop();
        }
        break;
    default:
        break;
    }

    Q_EMIT stateChanged();
}

