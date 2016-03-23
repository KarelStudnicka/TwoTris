#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Gfx.h"
#include "MenuMain.h"
#include "InputProcessor.h"
#include "Game.h"

MenuMain::MenuMain() {
	this->itemSelected = -1;
	this->itemCount = 4;
	this->items = new MenuMainItem[this->itemCount];
	this->items[0].label = "Nová hra";
	this->items[0].type = MenuMainItemType::newgame1p;
	this->items[1].label = "Hra pro dva";
	this->items[1].type = MenuMainItemType::newgame2p;
	this->items[2].label = "Nastavení";
	this->items[2].type = MenuMainItemType::options;
	this->items[3].label = "Konec";
	this->items[3].type = MenuMainItemType::quit;
	this->itemWidth = 200;
	this->itemHeight = 40;
	this->itemMargin = 10;
}


MenuMain::~MenuMain() {

}

void MenuMain::pushInput(ALLEGRO_EVENT *ev) {
	if (ev->type == InputProcessorActionType::KEY_DOWN) {
		int keyCode = ((InputProcessorInputData *)ev->user.data1)->keyDown.keycode;
		if ((keyCode == InputKeyType::KEY_P1_DOWN) || (keyCode == InputKeyType::KEY_P2_DOWN)) {
			if (this->itemSelected == -1) {
				this->itemSelected = 0;
			}
			else {
				this->itemSelected = (this->itemSelected + 1) % 4;
			}
		} else if ((keyCode == InputKeyType::KEY_P1_UP) || (keyCode == InputKeyType::KEY_P2_UP)) {
			if (this->itemSelected == -1) {
				this->itemSelected = this->itemCount - 1;
			}
			else {
				this->itemSelected = (this->itemSelected + this->itemCount - 1) % 4;
			}
		} else if (keyCode == InputKeyType::KEY_ENTER) {
			if (this->itemSelected != -1) {
				if (this->items[this->itemSelected].type == MenuMainItemType::quit) {
					game->setShouldQuit();
				} else if (this->items[this->itemSelected].type == MenuMainItemType::newgame1p) {
					game->startNewGame(1);
				} else if (this->items[this->itemSelected].type == MenuMainItemType::newgame2p) {
					game->startNewGame(2);
				}
			}
		} else if (keyCode == InputKeyType::KEY_ESCAPE) {
			if (game->getIsGameRunning()) game->continueGame();
		}
	}
}

void MenuMain::render() {
	int xPos = (int)((gfx->displayWidth - this->itemWidth) / 2);
	int yPos = (int)((gfx->displayHeight - this->itemHeight * this->itemCount - this->itemMargin * (this->itemCount - 1)) / 2);
	for (int ti = 0; ti < this->itemCount; ti++) {
		ALLEGRO_COLOR colBorder = al_map_rgb(100, 120, 100);
		ALLEGRO_COLOR colBackground = al_map_rgb(60, 60, 70);
		if (ti == this->itemSelected) {
			colBorder = al_map_rgb(50, 70, 50);
			colBackground = al_map_rgb(10, 10, 20);
		}
		gfx->putFilledRectangle(xPos, yPos + this->itemHeight * ti + this->itemMargin * ti, this->itemWidth, this->itemHeight, colBackground, colBorder);
		gfx->putText(this->items[ti].label, (int)(xPos + this->itemWidth / 2), (int)(yPos + this->itemHeight * ti + this->itemMargin * ti + this->itemHeight / 2 - 4), al_map_rgb(200, 230, 200), ALLEGRO_ALIGN_CENTRE);
	}
}
