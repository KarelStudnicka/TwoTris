#pragma once

#include<allegro5/allegro.h>

class MenuMain;
class GameSession;

enum GameStateType {
	menuMain,
	menuOptions,
	inGame,
	paused
};

class Game
{
private:
	int gameState;
	bool isGameRunning;
	bool isOnShouldQuit;

	MenuMain *menuMain;
	GameSession *gameSession;

public:
	Game();
	~Game();

	void setShouldQuit();
	void setShouldQuit(bool on);
	bool shouldQuit();
	void startNewGame(int playerCount = 1);
	void continueGame();
	void openMainMenu();
	bool getIsGameRunning();

	void pushInput(ALLEGRO_EVENT *ev);
	void tick(double tickDuration);

	void render();
	void renderMenuMain();
	void renderMenuOptions();
	void renderInGame();
	void renderPaused();
};

extern Game *game;