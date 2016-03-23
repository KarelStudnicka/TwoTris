#pragma once

#include<allegro5/allegro.h>

enum MenuMainItemType {
	newgame1p,
	newgame2p,
	options,
	quit
};

struct MenuMainItem {
	char *label;
	MenuMainItemType type;
};

class MenuMain
{
private:
	int itemSelected;
	MenuMainItem *items;
	int itemCount;
	int itemHeight;
	int itemWidth;
	int itemMargin;

public:
	MenuMain();
	~MenuMain();

	void pushInput(ALLEGRO_EVENT *ev);
	void render();
};

