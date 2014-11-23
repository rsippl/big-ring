#include "newvideowidget.h"

#include <QtCore/QUrl>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>

#include <QGlib/Connect>
#include <QGst/Bus>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/Event>
#include <QGst/Message>
#include <QGst/Query>
#include <QGst/Ui/GraphicsVideoSurface>
#include <QGst/Ui/GraphicsVideoWidget>

#include "clockgraphicsitem.h"
#include "simulation.h"

NewVideoWidget::NewVideoWidget( Simulation& simulation, QWidget *parent) :
    QGraphicsView(parent), _loadState(NONE)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setSceneRect(0, 0, 1600, 900);
    setViewport(new QWidget);
    _videoSurface = new QGst::Ui::GraphicsVideoSurface(this);
    _videoWidget = new QGst::Ui::GraphicsVideoWidget;

    _videoWidget->setSurface(_videoSurface);
    _videoWidget->setGeometry(scene->sceneRect());
    scene->addItem(_videoWidget);

    centerOn(_videoWidget);
    fitInView(_videoWidget);
    setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    addClock(simulation, scene);
}

void NewVideoWidget::addClock(Simulation &simulation, QGraphicsScene* scene)
{
    ClockGraphicsItem* item = new ClockGraphicsItem(simulation, this);
    item->setPos(800 - item->boundingRect().width() / 2, 0);
    scene->addItem(item);
}

NewVideoWidget::~NewVideoWidget()
{
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
    }
}

bool NewVideoWidget::isReadyToPlay()
{
    return (_loadState == DONE);
}

void NewVideoWidget::setRealLifeVideo(RealLifeVideo rlv)
{
    qDebug() << __FILE__ <<  "setting video to " << rlv.name();
    emit readyToPlay(false);
    _rlv = rlv;
    stop();

    QString uri = rlv.videoInformation().videoFilename();
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
            _pipeline->setProperty("video-sink", _videoSurface->videoSink());

            //watch the bus for messages
            QGst::BusPtr bus = _pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &NewVideoWidget::onBusMessage);
         } else {
            qWarning() << "Failed to create the pipeline";
        }
    }
    qDebug() << "setting uri";
    if (_pipeline) {
        qDebug() << "really setting uri" << uri;
        _pipeline->setProperty("uri", uri);
    }
    _pipeline->setState(QGst::StatePaused);

    _loadState = VIDEO_LOADING;
}

void NewVideoWidget::setCourseIndex(int index)
{

    if (!_rlv.isValid()) {
        return;
    }

    Course course = _rlv.courses()[qMax(0, index)];
    qDebug() << "VideoWidget: course set to " << course.name();
    switch(_loadState) {
    case NONE:
        qDebug() << "load state is none. Should not happen in NewVideoWidget::setCourse";
        break;
    case VIDEO_LOADED:
        _course = course;
        setDistance(_course.start());
        break;
    case DONE:
        _loadState = VIDEO_LOADED;
        _course = course;
        setDistance(_course.start());
        break;
    default:
        _course = course;
    }
}

void NewVideoWidget::stop()
{
    qDebug() << "stopped";
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        //once the pipeline stops, the bus is flushed so we will
        //not receive any StateChangedMessage about this.
        //so, to inform the ui, we have to emit this signal manually.
        Q_EMIT stateChanged();
    }
}

void NewVideoWidget::setDistance(float distance)
{
    if (_loadState == VIDEO_LOADED) {
        _currentFrame = _rlv.frameForDistance(distance);
        seekToStart();
        _loadState = SEEKING;
    } else if (_loadState == DONE) {
        quint32 targetFrame = _rlv.frameForDistance(distance);
        quint32 diff = targetFrame - _currentFrame;
        step(diff);
        if (diff >= 10) {
            qDebug() << "diff is too big:" << diff;
        }
        _currentFrame = targetFrame;
    }
}

void NewVideoWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    fitInView(_videoWidget);
    resizeEvent->accept();
}

void NewVideoWidget::enterEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void NewVideoWidget::leaveEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void NewVideoWidget::onBusMessage(const QGst::MessagePtr &message)
{
    switch (message->type()) {
    case QGst::MessageEos: //End of stream. We reached the end of the file.
        qDebug() << "eos";
        stop();
        break;
    case QGst::MessageError: //Some error occurred.
        qDebug() << "error";
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        stop();
        break;
    case QGst::MessageStateChanged: //The element in message->source() has changed state
        if (message->source() == _pipeline) {
            handlePipelineStateChange(message.staticCast<QGst::StateChangedMessage>());
        }
        break;
    case QGst::MessageAsyncDone:
        if (_loadState == VIDEO_LOADING) {
            QGst::DurationQueryPtr durationQuery = QGst::DurationQuery::create(QGst::FormatTime);
            _pipeline->query(durationQuery);
            quint64 nanoseconds = durationQuery->duration();
            qDebug() << "setting duration of video in rlv";
            _rlv.setDuration(nanoseconds / 1000);
            fitVideoWidget();
            _loadState = VIDEO_LOADED;
            if (_course.isValid()) {
                qDebug() << "course valid, setting start distance";
                setDistance(_course.start());
            }
        } else if (_loadState == SEEKING) {
            qDebug() << "seek ready";
            _loadState = DONE;
            emit readyToPlay(true);
        }
        break;
    default:
        break;
    }
}

void NewVideoWidget::handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm)
{
    switch (scm->newState()) {
     case QGst::StateChangeSuccess:
        // succes
        qDebug() << "success";
        break;
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

void NewVideoWidget::seekToStart()
{
    float seconds = _currentFrame / _rlv.videoInformation().frameRate();
    qDebug() << "need to seek to seconds: " << seconds;
    quint64 milliseconds = static_cast<quint64>(seconds * 1000);
    _pipeline->seek(QGst::FormatTime, QGst::SeekFlagFlush, QGst::ClockTime::fromMSecs(milliseconds));
}

void NewVideoWidget::step(int stepSize)
{
    QGst::EventPtr stepEvent = QGst::StepEvent::create(QGst::FormatBuffers, stepSize, 1.0, true, false);
    _pipeline->sendEvent(stepEvent);
}

void NewVideoWidget::fitVideoWidget()
{
    fitInView(_videoWidget);
//    _videoWidget->resize(size());
}

