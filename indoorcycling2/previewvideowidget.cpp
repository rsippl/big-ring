#include "previewvideowidget.h"
#include <QResizeEvent>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtOpenGL/QGLWidget>
#include <QGst/Clock>
#include <QGst/Allocator>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/Bus>
#include <QGst/Event>
#include <QGst/Query>
#include <QGst/Ui/GraphicsVideoSurface>
#include <QGst/Ui/GraphicsVideoWidget>
#include <QGst/Ui/VideoWidget>
#include <QGlib/Connect>

PreviewVideoWidget::PreviewVideoWidget(QWidget* parent):
    QWidget(parent), _stepTimer(new QTimer(this))
{
    QGraphicsScene* scene = new QGraphicsScene(this);
    _graphicsView = new QGraphicsView(scene, this);

    _graphicsView->setViewport(new QWidget);
    QGst::Ui::GraphicsVideoSurface *surface = new QGst::Ui::GraphicsVideoSurface(_graphicsView);
    QGst::Ui::GraphicsVideoWidget* videoWidget = new QGst::Ui::GraphicsVideoWidget;

    videoWidget->setSurface(surface);
    videoWidget->setGeometry(0, 0, 1600, 900);
    _videoWidget = videoWidget;
    scene->addItem(_videoWidget);

    _graphicsView->centerOn(_videoWidget);

    _graphicsView->fitInView(_videoWidget);
    _graphicsView->setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    _graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene->addText("Text");
    QBrush brush(Qt::red);
    QPen pen(brush, 20);
    scene->addEllipse(800 - 250, 900, 500, 200, pen);

    _videoSink = surface->videoSink();

    _stepTimer->setInterval(1000 / 30);
    connect(_stepTimer, &QTimer::timeout, this, &PreviewVideoWidget::step);
}

PreviewVideoWidget::~PreviewVideoWidget()
{
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
    }
}

void PreviewVideoWidget::setUri(QString uri)
{
    stop();
    //if uri is not a real uri, assume it is a file path
    if (uri.indexOf("://") < 0) {
        uri = QUrl::fromLocalFile(uri).toEncoded();
    }

    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }

    if (!_pipeline) {
        qDebug() << "creating new pipeline";
        _pipeline = QGst::ElementFactory::make("playbin").dynamicCast<QGst::Pipeline>();

        if (_pipeline) {
            _pipeline->setProperty("video-sink", _videoSink);

            //watch the bus for messages
            QGst::BusPtr bus = _pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &PreviewVideoWidget::onBusMessage);
        } else {
            qWarning() << "Failed to create the pipeline";
        }
    }

    if (_pipeline) {
        _pipeline->setProperty("uri", uri);
    }
}

void PreviewVideoWidget::play()
{
    _graphicsView->fitInView(_videoWidget);
    _videoWidget->resize(_graphicsView->size());
    if (_pipeline) {
        _pipeline->setState(QGst::StatePlaying);
//        _stepTimer->start();
    }
}

void PreviewVideoWidget::step()
{
    QGst::EventPtr stepEvent = QGst::StepEvent::create(QGst::FormatBuffers, 1, 1.0, true, false);
    _pipeline->sendEvent(stepEvent);
}

void PreviewVideoWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    _graphicsView->resize(resizeEvent->size());
    _graphicsView->fitInView(_videoWidget);
    _videoWidget->resize(_graphicsView->size());
    resizeEvent->accept();
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
    case QGst::MessageAsyncDone:
    {

        QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
        //This will create a temporary (cast to query).
        qDebug() << _pipeline->query(query);
        qDebug() << "seek" << query->position();
        bool result = _pipeline->seek(QGst::FormatTime, QGst::SeekFlagNone, 60000000000);
        qDebug() << "result = " << result;
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

