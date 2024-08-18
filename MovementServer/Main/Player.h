#ifndef __PLAYER_H__
#define __PLAYER_H__

#define MAP_WIDTH 15
#define MAP_HEIGHT 9

class Player
{
private:
	unsigned short x;
	unsigned short y;

	void Clear();

public:
	Player(unsigned short x, unsigned short y);
	~Player();

	void Draw();

	void SetPosition(unsigned short x, unsigned short y);
};

#endif