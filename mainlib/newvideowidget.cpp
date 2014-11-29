#include "newvideowidget.h"

// undefine emit, because we're using Glib::emit in here. Instead of
// Qt's emit, we'll use Q_EMIT in this file.
#undef emit

#include <QtCore/QUrl>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>
#include <QtGui/QResizeEvent>

#include <QGlib/Connect>
#include <QGlib/Signal>
#include <QGst/Bus>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/Event>
#include <QGst/Message>
#include <QGst/Query>

#include "clockgraphicsitem.h"
#include "sensoritem.h"
#include "simulation.h"


NewVideoWidget::NewVideoWidget( Simulation& simulation, QWidget *parent) :
    QGraphicsView(parent), _loadState(NONE)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setSceneRect(0, 0, 4096, 2048);
    setViewport(new QGLWidget);

    setUpVideoSink();
    setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheNone);

    addClock(simulation, scene);
    addWattage(simulation, scene);
    addHeartRate(simulation, scene);
    addCadence(simulation, scene);
    addSpeed(simulation, scene);
    addDistance(simulation, scene);
    addGrade(simulation, scene);
}

void NewVideoWidget::addClock(Simulation &simulation, QGraphicsScene* scene)
{
    _clockItem = new ClockGraphicsItem(simulation);
    QPointF scenePosition = mapToScene(width() / 2, 0);
    _clockItem->setPos(scenePosition.x() - (_clockItem->boundingRect().width() / 2), scenePosition.y());
    scene->addItem(_clockItem);


}

void NewVideoWidget::addWattage(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* wattageItem = new SensorItem("W");
    scene->addItem(wattageItem);
    connect(&simulation.cyclist(), &Cyclist::powerChanged, this, [wattageItem](float power) {
       wattageItem->setValue(QVariant::fromValue(power));
    });
    _wattageItem = wattageItem;
}

void NewVideoWidget::addCadence(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* cadenceItem = new SensorItem("RPM");
    scene->addItem(cadenceItem);
    connect(&simulation.cyclist(), &Cyclist::cadenceChanged, this, [cadenceItem](quint8 cadence) {
       cadenceItem->setValue(QVariant::fromValue(static_cast<int>(cadence)));
    });
    _cadenceItem = cadenceItem;
}

void NewVideoWidget::addSpeed(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* speedItem = new SensorItem("KM/H", "000.0");
    scene->addItem(speedItem);
    connect(&simulation.cyclist(), &Cyclist::speedChanged, this, [speedItem](float speed) {
       speedItem->setValue(QVariant::fromValue(QString("%1").arg(speed * 3.6, 1, 'f', 1)));
    });
    _speedItem = speedItem;
}

void NewVideoWidget::addGrade(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* gradeItem = new SensorItem("%", "-00.0");
    scene->addItem(gradeItem);
    connect(&simulation, &Simulation::slopeChanged, this, [gradeItem](float grade) {
       gradeItem->setValue(QVariant::fromValue(QString("%1").arg(grade, 1, 'f', 1)));
    });
    _gradeItem = gradeItem;
}

void NewVideoWidget::addDistance(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* distanceItem = new SensorItem("M", "00000");
    scene->addItem(distanceItem);
    connect(&simulation.cyclist(), &Cyclist::distanceChanged, this, [distanceItem](float distance) {
       distanceItem->setValue(QVariant::fromValue(static_cast<int>(distance)));
    });
    _distanceItem = distanceItem;
}

void NewVideoWidget::addHeartRate(Simulation &simulation, QGraphicsScene *scene)
{
    SensorItem* heartRateItem = new SensorItem("BPM", "000");
    scene->addItem(heartRateItem);
    connect(&simulation.cyclist(), &Cyclist::heartRateChanged, this, [heartRateItem](quint8 heartRate) {
       heartRateItem->setValue(QVariant::fromValue(static_cast<int>(heartRate)));
    });
    _heartRateItem = heartRateItem;
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
    Q_EMIT(readyToPlay(false));
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
            _pipeline->setProperty("video-sink", _videoSink);

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
    QPointF scenePosition = mapToScene(width() / 2, 0);
    _clockItem->setPos(scenePosition.x() - (_clockItem->boundingRect().width() / 2), scenePosition.y());
    scenePosition = mapToScene(0, height() /2);
    _wattageItem->setPos(scenePosition);
    scenePosition = mapToScene(0, height() /3);
    _heartRateItem->setPos(scenePosition);
    scenePosition = mapToScene(0, 2 * height() /3);
    _cadenceItem->setPos(scenePosition);
    scenePosition = mapToScene(width(), height() / 3);
    _speedItem->setPos(scenePosition.x() - _speedItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), height() /2);
    _distanceItem->setPos(scenePosition.x() - _distanceItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), 2 * height() /3);
    _gradeItem->setPos(scenePosition.x() - _gradeItem->boundingRect().width(), scenePosition.y());
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

/*!
 *
 * \brief Draw background by telling the video sink to update the complete background.
 * \param painter the painter used for drawing.
 */
void NewVideoWidget::drawBackground(QPainter *painter, const QRectF &)
{
    const QRectF r = rect();
    QGlib::emit<void>(_videoSink, "paint", (void*) painter, r.x(), r.y(), r.width(), r.height());
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
            _loadState = VIDEO_LOADED;
            if (_course.isValid()) {
                qDebug() << "course valid, setting start distance";
                setDistance(_course.start());
            }
        } else if (_loadState == SEEKING) {
            qDebug() << "seek ready";
            _loadState = DONE;
            Q_EMIT(readyToPlay(true));
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

/*!
 * setup the video sink. This tries to create a gstreamer "qt5glvideosink".
 * If this succeeds, it will handle the OpenGL context to this element, so
 * it can be used to draw the video.
 *
 * NewVideoWidget::onVideoUpdate() will be called when there is something to draw.
 */
void NewVideoWidget::setUpVideoSink()
{
    QGLWidget* paintWidget = qobject_cast<QGLWidget*>(viewport());
    _videoSink = QGst::ElementFactory::make("qt5glvideosink");
    if (!_videoSink.isNull()) {
        paintWidget->makeCurrent();
        _videoSink->setProperty("glcontext", (void*) QGLContext::currentContext());
        paintWidget->doneCurrent();
        if (_videoSink->setState(QGst::StateReady) != QGst::StateChangeSuccess) {
            _videoSink.clear();
        }
        QGlib::connect(_videoSink, "update",
                       this,
                       &NewVideoWidget::onVideoUpdate);
    }
}

/*!
 * \brief update video by letting the viewport update.
 */
void NewVideoWidget::onVideoUpdate()
{
    viewport()->update();
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
    if (stepSize > 0) {
        QGst::EventPtr stepEvent = QGst::StepEvent::create(QGst::FormatBuffers, stepSize, 1.0, true, false);
        _pipeline->sendEvent(stepEvent);
    }
}


