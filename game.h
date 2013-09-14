#ifndef GAME_H
#define GAME_H

#include <QTime>
#include <QObject>

class Game : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QTime gameTime READ gameTime)
public:
	explicit Game(QObject *parent = 0);
	
	QTime gameTime() const;

public slots:
	Q_INVOKABLE void start();
private:
	QTime _startTime;
};

#endif // GAME_H
