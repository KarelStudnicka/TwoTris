#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Gfx.h"
#include "MenuMain.h"
#include "GameSession.h"
#include "Game.h"

Game::Game() {
	this->gameState = GameStateType::menuMain;
	this->isGameRunning = false;
	this->menuMain = new MenuMain();
	this->gameSession = new GameSession();
	this->isOnShouldQuit = false;
}

Game::~Game() {
}

void Game::setShouldQuit() {
	this->isOnShouldQuit = true;
}

void Game::setShouldQuit(bool on) {
	this->isOnShouldQuit = on;
}

bool Game::shouldQuit() {
	return this->isOnShouldQuit;
}

void Game::startNewGame(int playerCount) {
	this->gameSession->reset(playerCount);
	this->gameState = GameStateType::inGame;
	this->isGameRunning = true;
}

void Game::continueGame() {
	if (this->getIsGameRunning()) this->gameState = GameStateType::inGame;
}

void Game::openMainMenu() {
	this->gameState = GameStateType::menuMain;
}

bool Game::getIsGameRunning() {
	return this->isGameRunning;
}

void Game::pushInput(ALLEGRO_EVENT *ev) {
	if (this->gameState == GameStateType::menuMain) {
		this->menuMain->pushInput(ev);
	} else if (this->gameState == GameStateType::menuOptions) {
	} else if (this->gameState == GameStateType::inGame) {
		this->gameSession->pushInput(ev);
	} else if (this->gameState == GameStateType::paused) {
	}
}

void Game::tick(double tickDuration) {
	if (this->gameState == GameStateType::menuMain) {
	} else if (this->gameState == GameStateType::menuOptions) {
	} else if (this->gameState == GameStateType::inGame) {
		this->gameSession->tick(tickDuration);
	} else if (this->gameState == GameStateType::paused) {
	}
}

void Game::render() {
	if (this->gameState == GameStateType::menuMain) {
		this->renderMenuMain();
	} else if (this->gameState == GameStateType::menuOptions) {
		this->renderMenuOptions();
	} else if (this->gameState == GameStateType::inGame) {
		this->renderInGame();
	} else if (this->gameState == GameStateType::paused) {
		this->renderPaused();
	}
}

void Game::renderMenuMain() {
	gfx->startDrawing();
	gfx->clearBackground(al_map_rgb(0, 0, 0));
	if (this->getIsGameRunning()) {
		this->gameSession->render();
		gfx->putFilledRectangle(0, 0, gfx->displayWidth, gfx->displayHeight, al_map_rgba(0, 0, 0, 120), al_map_rgba(0, 0, 0, 120));
	}
	this->menuMain->render();
	gfx->endDrawing();
	gfx->render();
}

void Game::renderMenuOptions() {
	gfx->startDrawing();
	gfx->clearBackground(al_map_rgb(0, 0, 0));
	gfx->endDrawing();
	gfx->render();
}

void Game::renderInGame() {
	gfx->startDrawing();
	gfx->clearBackground(al_map_rgb(0, 0, 0));
	this->gameSession->render();
	gfx->endDrawing();
	gfx->render();
}

void Game::renderPaused() {
	gfx->startDrawing();
	gfx->clearBackground(al_map_rgb(0, 0, 0));
	gfx->endDrawing();
	gfx->render();
}

Game *game;