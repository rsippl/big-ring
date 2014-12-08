#include "newvideowidget.h"

// undefine emit, because we're using Glib::emit in here. Instead of
// Qt's emit, we'll use Q_EMIT in this file.
#undef emit

#include <QtCore/QtDebug>
#include <QtCore/QUrl>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>
#include <QtGui/QResizeEvent>

#include "clockgraphicsitem.h"
#include "profileitem.h"
#include "sensoritem.h"
#include "simulation.h"


NewVideoWidget::NewVideoWidget( Simulation& simulation, QWidget *parent) :
    QGraphicsView(parent), _loadState(NONE), _bus(nullptr), _pipeline(nullptr)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setSceneRect(0, 0, 2048, 2048);
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

    _profileItem = new ProfileItem(simulation);
    scene->addItem(_profileItem);

    _busTimer = new QTimer(this);
    _busTimer->setInterval(20);
    connect(_busTimer, &QTimer::timeout, this, &NewVideoWidget::pollBus);
}

void NewVideoWidget::addClock(Simulation &simulation, QGraphicsScene* scene)
{
    _clockItem = new ClockGraphicsItem(simulation);
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
    SensorItem* distanceItem = new SensorItem("M", "000000");
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

void NewVideoWidget::pollBus()
{
    if (_bus) {
        GstMessage* message;
        g_object_ref(_bus);

        while((message = gst_bus_pop(_bus))) {
            onBusMessage(_bus, message, this);
        }

        g_object_unref(_bus);
    }
}

NewVideoWidget::~NewVideoWidget()
{
    if (_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        g_object_unref(_pipeline);
    } else {
        if (_videoSink) {
            gst_element_set_state(_videoSink, GST_STATE_NULL);
            g_object_unref(_videoSink);
        }
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
    _profileItem->setRlv(rlv);
    stop();

    QString uri = rlv.videoInformation().videoFilename();
    if (uri.indexOf("://") < 0) {
        uri = QUrl::fromLocalFile(uri).toEncoded();
    }

    if (_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        g_object_unref(_pipeline);
        _pipeline = nullptr;
    }

    if (!_pipeline) {
        qDebug() << "creating new pipeline";
        _pipeline = gst_element_factory_make("playbin", "playbin");
        //        _pipeline = QGst::ElementFactory::make("playbin").dynamicCast<QGst::Pipeline>();

        if (_pipeline) {
            g_object_set(_pipeline, "video-sink", _videoSink, NULL);
            _bus = gst_element_get_bus(_pipeline);
            _busTimer->start();
        } else {
            qWarning() << "Failed to create the pipeline";
        }
    }
    qDebug() << "setting uri";
    if (_pipeline) {
        qDebug() << "really setting uri" << uri;
        g_object_set(_pipeline, "uri", uri.toStdString().c_str(), NULL);
    }
    gst_element_set_state(_pipeline, GST_STATE_PAUSED);

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
        gst_element_set_state(_pipeline, GST_STATE_NULL);
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
    scenePosition = mapToScene(0, height() /8);
    _wattageItem->setPos(scenePosition);
    scenePosition = mapToScene(0, 2* height() /8);
    _heartRateItem->setPos(scenePosition);
    scenePosition = mapToScene(0, 3 * height() /8);
    _cadenceItem->setPos(scenePosition);
    scenePosition = mapToScene(width(), 1 * height() / 8);
    _speedItem->setPos(scenePosition.x() - _speedItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), 2 * height() /8);
    _distanceItem->setPos(scenePosition.x() - _distanceItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), 3 * height() / 8);
    _gradeItem->setPos(scenePosition.x() - _gradeItem->boundingRect().width(), scenePosition.y());
    resizeEvent->accept();

    _profileItem->setSize(QSize(width() * 6 / 8, height() * 1 / 8));
    scenePosition = mapToScene(width() * 1 / 8, height() * 27 / 32);
    _profileItem->setPos(scenePosition);
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
    g_signal_emit_by_name(_videoSink, "paint", painter, r.x(), r.y(), r.width(), r.height(), NULL);
}

void NewVideoWidget::onBusMessage(GstBus *bus, GstMessage *msg, NewVideoWidget *context)
{
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS: //End of stream. We reached the end of the file.
        qDebug() << "eos";
        context->stop();
        break;
    case GST_MESSAGE_ERROR:
    {
        GError *err;
        gchar *debug;

        gst_message_parse_error (msg, &err, &debug);
        QString errorString(err->message);
        qDebug() << "error" << errorString;
        g_error_free(err);
        g_free(debug);
        context->stop();
    }
        break;
    case GST_MESSAGE_ASYNC_DONE:
        context->handleAsyncDone();

        break;
    default:
        break;
    }
}

void NewVideoWidget::handleAsyncDone()
{
    if (_loadState == VIDEO_LOADING) {
        gint64 nanoSeconds;
        gst_element_query_duration(_pipeline, GST_FORMAT_TIME, &nanoSeconds);
        qDebug() << "setting duration of video in rlv";
        _rlv.setDuration(nanoSeconds / 1000);
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
    _videoSink = gst_element_factory_make("qt5glvideosink", "qt5glvideosink");
    if (_videoSink) {
        paintWidget->makeCurrent();
        g_object_set(_videoSink, "glcontext", QGLContext::currentContext(), NULL);
        paintWidget->doneCurrent();
        gst_element_set_state(_videoSink, GST_STATE_READY);

        qDebug() << "attaching signal";
        g_signal_connect(_videoSink, "update", G_CALLBACK(NewVideoWidget::onVideoUpdate), this);
    }
}

/*!
 * \brief update video by letting the viewport update.
 */
void NewVideoWidget::onVideoUpdate(GObject* src, guint, NewVideoWidget* context)
{
    context->viewport()->update();
}

void NewVideoWidget::seekToStart()
{
    float seconds = _currentFrame / _rlv.videoInformation().frameRate();
    qDebug() << "need to seek to seconds: " << seconds;
    quint64 milliseconds = static_cast<quint64>(seconds * 1000);
    gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, milliseconds * GST_MSECOND);
}

void NewVideoWidget::step(int stepSize)
{
    if (stepSize > 0) {
        gst_element_send_event(_pipeline, gst_event_new_step(GST_FORMAT_BUFFERS, stepSize, 1.0, true, false));
    }
}


