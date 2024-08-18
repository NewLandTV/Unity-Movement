#include <Windows.h>
#include "Utils.h"

void Utils::Gotoxy(unsigned short x, unsigned short y)
{
	COORD position = { x, y };

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
}