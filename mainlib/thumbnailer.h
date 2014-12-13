#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QtCore/QDir>
#include <QtGui/QPixmap>
#include "reallifevideo.h"

class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(QObject* parent = 0);
    explicit Thumbnailer(const QString& cacheDirectory, QObject *parent = 0);

    QString cacheFilePathFor(const RealLifeVideo& rlv);
    static QDir thumbnailDirectory();
signals:

public slots:
    /**
     * @brief Create a thumbnail for the rlv.
     * @param rlv the rlv to create the thumbnail for.
     */
    void createThumbnailFor(const RealLifeVideo &rlv);


private:
    /*!
     * \brief Create the cache directory in which the thumbnails will be stored, if it does not exist yet.
     */
    void createCacheDirectoryIfNotExists();
    bool doesThumbnailExistsFor(const RealLifeVideo& rlv);

    QDir _cacheDirectory;
};

#endif // THUMBNAILER_H
