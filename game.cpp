#include "game.h"
#include <QtDebug>

Game::Game(QObject *parent) :
	QObject(parent)
{
}

void Game::start()
{
	_startTime.start();
}

QTime Game::gameTime() const
{
	int elapsed = _startTime.elapsed();
	return QTime(0,0,0).addMSecs(elapsed);
}
