#include <iostream>
#include "Player.h"
#include "Utils.h"

void Player::Clear()
{
	if (x == 0 && y == 0)
	{
		return;
	}

	Utils::Gotoxy(((MAP_WIDTH >> 1) + x) * 2, (MAP_HEIGHT >> 1) - y);

	std::cout << " ";
}

Player::Player(unsigned short x, unsigned short y) : x(x), y(y)
{
	Draw();
}

Player::~Player()
{

}

void Player::Draw()
{
	Utils::Gotoxy(((MAP_WIDTH >> 1) + x) * 2, (MAP_HEIGHT >> 1) - y);

	std::cout << "@";
}

void Player::SetPosition(unsigned short x, unsigned short y)
{
	Clear();

	this->x = x;
	this->y = y;

	Draw();
}