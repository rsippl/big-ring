#ifndef INFORMATIONBOXGRAPHICSITEM_H
#define INFORMATIONBOXGRAPHICSITEM_H

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtWidgets/QGraphicsItem>

class InformationBoxGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit InformationBoxGraphicsItem(QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void setImageFileInfo(const QFileInfo &imageFileInfo);
    void setText(const QString &text);
private:
    QGraphicsPixmapItem *_pixmapItem;
    QGraphicsTextItem *_textItem;

};

#endif // INFORMATIONBOXGRAPHICSITEM_H
