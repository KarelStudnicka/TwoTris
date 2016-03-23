#include <iostream>
#include <random>
#include <allegro5/allegro.h>
#include "InputProcessor.h"
#include "Gfx.h"
#include "Game.h"
#include "GameSession.h"

GameSession::GameSession() {
	this->grid.resize(this->GRID_HEIGHT);
	for (int row = 0; row < this->GRID_HEIGHT; row++) {
		this->grid[row].resize(GRID_WIDTH);
	}
	this->blockColors.push_back(al_map_rgb(0, 0, 0));
	this->blockColors.push_back(al_map_rgb(255, 0, 0));
	this->blockColors.push_back(al_map_rgb(0, 255, 0));
	this->blockColors.push_back(al_map_rgb(0, 0, 255));
	this->blockColors.push_back(al_map_rgb(255, 255, 0));
	this->blockColors.push_back(al_map_rgb(0, 255, 255));
	this->blockColors.push_back(al_map_rgb(255, 0, 255));
	this->prepareShapes();
	this->prepareRotations();
	this->players.resize(2);
}


GameSession::~GameSession() {
}

void GameSession::reset(int playerCount) {
	this->playerCount = playerCount;
	for (int row = 0; row < this->GRID_HEIGHT; row++) {
		for (int col = 0; col < this->GRID_WIDTH; col++) {
			this->grid[row][col] = 0;
		}
	}
	this->resetPlayer(0);
	this->resetPlayer(1);
	this->resetSpawners(playerCount);
	this->time = 0.0;
}

void GameSession::pushInput(ALLEGRO_EVENT *ev) {
	if (ev->type == InputProcessorActionType::KEY_DOWN) {
		int keyCode = ((InputProcessorInputData *)ev->user.data1)->keyDown.keycode;
		if (keyCode == InputKeyType::KEY_ESCAPE) {
			game->openMainMenu();
		} else if (keyCode == InputKeyType::KEY_P1_LEFT) {
			if (this->players[0].blockSpawned) this->setIntent(0, BlockActionIntent::moveleft);
		} else if (keyCode == InputKeyType::KEY_P1_RIGHT) {
			if (this->players[0].blockSpawned) this->setIntent(0, BlockActionIntent::moveright);
		} else if (keyCode == InputKeyType::KEY_P1_DOWN) {
			if (this->players[0].blockSpawned) this->setIntent(0, BlockActionIntent::movedown);
		} else if (keyCode == InputKeyType::KEY_P1_UP) {
			if (this->players[0].blockSpawned) this->setIntent(0, BlockActionIntent::rotatecw);
		} else if (this->playerCount == 2) {
			if (keyCode == InputKeyType::KEY_P2_LEFT) {
				if (this->players[1].blockSpawned) this->setIntent(1, BlockActionIntent::moveleft);
			} else if (keyCode == InputKeyType::KEY_P2_RIGHT) {
				if (this->players[1].blockSpawned) this->setIntent(1, BlockActionIntent::moveright);
			} else if (keyCode == InputKeyType::KEY_P2_DOWN) {
				if (this->players[1].blockSpawned) this->setIntent(1, BlockActionIntent::movedown);
			} else if (keyCode == InputKeyType::KEY_P2_UP) {
				if (this->players[1].blockSpawned) this->setIntent(1, BlockActionIntent::rotatecw);
			}
		}
	}
}

void GameSession::tick(double timePassed) {
	// increase passed time
	this->newTime += timePassed;

	// spawn block
	for (int i = 0; i < this->playerCount; i++) {
		if (this->shouldSpawnBlock(i)) {
			this->spawnBlock(i);
		}
	}

	// drop block
	for (int i = 0; i < this->playerCount; i++) {
		if (this->players[i].blockSpawned) {
			if (this->shouldDrop(i)) {
				if (this->canDrop(i)) {
					this->drop(i);
				} else {
					this->assimilate(i);
					this->removeFullLines();
				}
			}
		}
	}

	// solve intents
	for (int i = 0; i < this->playerCount; i++) {
		if (this->players[i].blockSpawned) {
			if (this->players[i].intent != BlockActionIntent::noaction) {
				this->players[i].intentBlockX = this->players[i].blockX;
				this->players[i].intentBlockY = this->players[i].blockY;
				this->players[i].intentBlockRotation = this->players[i].blockRotation;
				if (this->players[i].intent == BlockActionIntent::moveleft) {
					this->players[i].intentBlockX -= 1;
				}
				if (this->players[i].intent == BlockActionIntent::moveright) {
					this->players[i].intentBlockX += 1;
				}
				if (this->players[i].intent == BlockActionIntent::movedown) {
					this->players[i].intentBlockY += 1;
				}
				if (this->players[i].intent == BlockActionIntent::rotatecw) {
					this->players[i].intentBlockRotation = (this->players[i].intentBlockRotation + 1) % 4;
				}
				// check collisions
				if (
					!this->collideWithGrid(i)
					) {
					this->players[i].blockX = this->players[i].intentBlockX;
					this->players[i].blockY = this->players[i].intentBlockY;
					this->players[i].blockRotation = this->players[i].intentBlockRotation;
				}
				this->players[i].intent = BlockActionIntent::noaction;
			}
		}
	}

	// reset intents
	this->resetIntents();

	// increase passed last time
	this->time += this->newTime;
}

void GameSession::removeFullLines() {
	for (int ty = 0; ty < this->GRID_HEIGHT; ty++) {
		bool lineIsFull = true;
		for (int tx = 0; tx < this->GRID_WIDTH; tx++) {
			if (!this->grid[ty][tx]) lineIsFull = false;
		}
		if (lineIsFull) this->removeLine(ty);
	}
}

void GameSession::removeLine(int y) {
	if (y == 0) {
		for (int tx = 0; tx < this->GRID_WIDTH; tx++) this->grid[0][tx] = 0;
	} else {
		for (int ty = y - 1; ty >= 0; ty--) {
			for (int tx = 0; tx < this->GRID_WIDTH; tx++) {
				this->grid[ty + 1][tx] = this->grid[ty][tx];
			}
		}
	}
}

bool GameSession::collideWithGrid(int idPlayer) {
	int sizeX = 3;
	int sizeY = 3;
	ShapeStruct shapeStruct = this->shapes[this->players[idPlayer].blockShape];
	int idRot = this->players[idPlayer].intentBlockRotation;
	Rotation rot = this->rotations3[idRot];
	if (shapeStruct.size == 4) {
		sizeX = 4;
		sizeY = 4;
		rot = this->rotations4[idRot];
	}
	for (int ty = 0; ty < sizeY; ty++) {
		for (int tx = 0; tx < sizeX; tx++) {
			int rotIndex = rot[ty * sizeX + tx];
			int blockValue = shapeStruct.shape[rotIndex];
			if (
				blockValue &&
				(
					this->players[idPlayer].intentBlockY + ty < 0 ||
					this->players[idPlayer].intentBlockY + ty >= this->GRID_HEIGHT ||
					this->players[idPlayer].intentBlockX + tx < 0 ||
					this->players[idPlayer].intentBlockX + tx >= this->GRID_WIDTH ||
					this->grid[this->players[idPlayer].intentBlockY + ty][this->players[idPlayer].intentBlockX + tx]
					)
				) return true;
		}
	}
	return false;
}

bool GameSession::shouldDrop(int idPlayer) {
	if (this->players[idPlayer].nextDrop <= this->newTime) {
		return true;
	} else {
		return false;
	}
}

bool GameSession::canDrop(int idPlayer) {
	int sizeX = 3;
	int sizeY = 3;
	ShapeStruct shapeStruct = this->shapes[this->players[idPlayer].blockShape];
	int idRot = this->players[idPlayer].blockRotation;
	Rotation rot = this->rotations3[idRot];
	if (shapeStruct.size == 4) {
		sizeX = 4;
		sizeY = 4;
		rot = this->rotations4[idRot];
	}
	for (int ty = 0; ty < sizeY; ty++) {
		for (int tx = 0; tx < sizeX; tx++) {
			int rotIndex = rot[ty * sizeX + tx];
			int blockValue = shapeStruct.shape[rotIndex];
			if (
				blockValue && 
				(
					this->players[idPlayer].blockY + 1 + ty < 0 ||
					this->players[idPlayer].blockY + 1 + ty >= this->GRID_HEIGHT ||
					this->players[idPlayer].blockX + tx < 0 ||
					this->players[idPlayer].blockX + tx >= this->GRID_WIDTH ||
					this->grid[this->players[idPlayer].blockY + 1 + ty][this->players[idPlayer].blockX + tx]
				)
			) return false;
		}
	}
	return true;
}

void GameSession::drop(int idPlayer) {
	this->players[idPlayer].blockY++;
	this->players[idPlayer].nextDrop = this->newTime + this->players[idPlayer].dropSpeed;
}

void GameSession::assimilate(int idPlayer) {
	int sizeX = 3;
	int sizeY = 3;
	ShapeStruct shapeStruct = this->shapes[this->players[idPlayer].blockShape];
	int idRot = this->players[idPlayer].blockRotation;
	Rotation rot = this->rotations3[idRot];
	if (shapeStruct.size == 4) {
		sizeX = 4;
		sizeY = 4;
		rot = this->rotations4[idRot];
	}
	for (int ty = 0; ty < sizeY; ty++) {
		for (int tx = 0; tx < sizeX; tx++) {
			int rotIndex = rot[ty * sizeX + tx];
			int blockValue = shapeStruct.shape[rotIndex];
			if (blockValue) {
				if (
					this->players[idPlayer].blockY + ty >= 0 &&
					this->players[idPlayer].blockY + ty < this->GRID_HEIGHT &&
					this->players[idPlayer].blockX + tx >= 0 &&
					this->players[idPlayer].blockX + tx < this->GRID_WIDTH
					) {
					this->grid[this->players[idPlayer].blockY + ty][this->players[idPlayer].blockX + tx] = blockValue;
				}
			}
		}
	}
	this->players[idPlayer].blockSpawned = false;
	this->players[idPlayer].nextSpawn = this->newTime;
}

void GameSession::render() {
	/*int xPos = (int)((gfx->displayWidth - this->itemWidth) / 2);
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
	}*/
	this->renderGrid(50, 50);
	for (int i = 0; i < this->playerCount; i++) {
		if (this->players[i].blockSpawned) {
			this->renderShape(50, 50, this->players[i].blockX, this->players[i].blockY, this->players[i].blockShape, this->players[i].blockRotation, this->players[i].blockColor);
		}
	}
}

void GameSession::renderGrid(int x, int y) {
	int borderWidth = 4;
	gfx->putFilledRectangle(
		x - borderWidth, y - borderWidth,
		this->GRID_WIDTH * this->BLOCK_WIDTH + (this->GRID_WIDTH - 1) * this->BLOCK_SPACING + borderWidth * 2,
		this->GRID_HEIGHT * this->BLOCK_HEIGHT + (this->GRID_HEIGHT - 1) * this->BLOCK_SPACING + borderWidth * 2,
		al_map_rgb(10, 10, 10), al_map_rgb(80, 80, 80)
		);
	for (int row = 0; row < this->GRID_HEIGHT; row++) {
		for (int col = 0; col < this->GRID_WIDTH; col++) {
			if (this->grid[row][col]) {
				gfx->putFilledRectangle(
					x + col * this->BLOCK_WIDTH + col * this->BLOCK_SPACING,
					y + row * this->BLOCK_HEIGHT + row * this->BLOCK_SPACING,
					this->BLOCK_WIDTH, this->BLOCK_HEIGHT,
					this->blockColors[this->grid[row][col]],
					al_map_rgba(0, 0, 0, 120)
					);
			}
		}
	}
}

void GameSession::renderShape(int baseX, int baseY, int x, int y, int idShape, int idRotation, int idColor) {
	Shape shape = this->shapes[idShape].shape;
	Rotation rot = this->rotations3[idRotation];
	if (this->shapes[idShape].size == 4) rot = this->rotations4[idRotation];
	ALLEGRO_COLOR col = this->blockColors[idColor];
	int sx = 3;
	int sy = 3;
	if (this->shapes[idShape].size == 4) {
		sx = 4;
		sy = 4;
	}
	for (int ty = 0; ty < sy; ty++) {
		for (int tx = 0; tx < sx; tx++) {
			int blockValue = shape[rot[ty * sx + tx]];
			if (blockValue) {
				gfx->putFilledRectangle(
					baseX + (tx + x) * this->BLOCK_WIDTH + (tx + x) * this->BLOCK_SPACING,
					baseY + (ty + y) * this->BLOCK_HEIGHT + (ty + y) * this->BLOCK_SPACING,
					this->BLOCK_WIDTH, this->BLOCK_HEIGHT,
					col,
					al_map_rgba(0, 0, 0, 120)
					);
			}
		}
	}
}

void GameSession::prepareShapes() {
	ShapeStruct shapeStruct;
	this->shapes.clear();
	shapeStruct.shape = {
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0
	};
	shapeStruct.size = 4;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		1, 0, 0,
		1, 0, 0,
		1, 1, 0
	};
	shapeStruct.size = 3;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		0, 1, 0,
		0, 1, 0,
		1, 1, 0
	};
	shapeStruct.size = 3;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	};
	shapeStruct.size = 4;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		0, 0, 0,
		0, 1, 1,
		1, 1, 0
	};
	shapeStruct.size = 3;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		0, 0, 0,
		1, 1, 0,
		0, 1, 1
	};
	shapeStruct.size = 3;
	this->shapes.push_back(shapeStruct);
	shapeStruct.shape = {
		0, 1, 0,
		1, 1, 1,
		0, 0, 0
	};
	shapeStruct.size = 3;
	this->shapes.push_back(shapeStruct);
}

void GameSession::prepareRotations() {
	Rotation rot;
	rot = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8
	};
	this->rotations3.push_back(rot);
	rot = {
		6, 3, 0,
		7, 4, 1,
		8, 5, 2
	};
	this->rotations3.push_back(rot);
	rot = {
		8, 7, 6,
		5, 4, 3,
		2, 1, 0
	};
	this->rotations3.push_back(rot);
	rot = {
		2, 5, 8,
		1, 4, 7,
		0, 3, 6
	};
	this->rotations3.push_back(rot);
	rot = {
		 0, 1, 2, 3,
		 4, 5, 6, 7,
		 8, 9,10,11,
		12,13,14,15
	};
	this->rotations4.push_back(rot);
	rot = {
		12, 8, 4, 0,
		13, 9, 5, 1,
		14,10, 6, 2,
		15,11, 7, 3
	};
	this->rotations4.push_back(rot);
	rot = {
		 15,14,13,12,
		 11,10, 9, 8,
		  7, 6, 5, 4,
		  3, 2, 1, 0
	};
	this->rotations4.push_back(rot);
	rot = {
		 3, 7,11,15,
		 2, 6,10,14,
		 1, 5, 9,13,
		 0, 4, 8,12
	};
	this->rotations4.push_back(rot);
}

int GameSession::getRandomColor() {
	static int colorCount;
	static std::random_device rseed;
	static std::mt19937 *rgen;
	static std::uniform_int_distribution<int> *idist;

	if (!colorCount) {
		colorCount = this->blockColors.size();
		rgen = new std::mt19937(rseed());
		idist = new std::uniform_int_distribution<int>(1, colorCount - 1);
	}

	return (*idist)(*rgen);
}

int GameSession::getRandomShape() {
	static int shapeCount;
	static std::random_device rseed;
	static std::mt19937 *rgen;
	static std::uniform_int_distribution<int> *idist;

	if (!shapeCount) {
		shapeCount = this->shapes.size();
		rgen = new std::mt19937(rseed());
		idist = new std::uniform_int_distribution<int>(0, shapeCount - 1);
	}

	return (*idist)(*rgen);
}

void GameSession::resetPlayer(int idPlayer) {
	this->players[idPlayer].blockSpawned = false;
	this->players[idPlayer].spawnX = 3;
	this->players[idPlayer].spawnY = 0;
	this->players[idPlayer].nextBlockColor = this->getRandomColor();
	this->players[idPlayer].nextBlockShape = this->getRandomShape();
	this->players[idPlayer].nextSpawn = 0;
	this->players[idPlayer].score = 0;
	this->players[idPlayer].dropSpeed = 0.4;
	this->players[idPlayer].blockX = 0;
	this->players[idPlayer].blockY = 0;
	this->players[idPlayer].blockColor = 0;
	this->players[idPlayer].blockRotation = 0;
	this->players[idPlayer].blockShape = 0;
}

void GameSession::resetSpawners(int playerCount) {
	if (playerCount == 1) {
		this->players[0].spawnX = 3;
		this->players[0].spawnY = 0;
	} else {
		this->players[0].spawnX = 1;
		this->players[0].spawnY = 0;
		this->players[1].spawnX = 5;
		this->players[1].spawnY = 0;
	}
}

bool GameSession::shouldSpawnBlock(int idPlayer) {
	if ((this->players[idPlayer].nextSpawn != -1) && (this->players[idPlayer].nextSpawn <= this->newTime)) {
		return true;
	} else return false;
}

void GameSession::spawnBlock(int idPlayer) {
	this->players[idPlayer].blockX = this->players[idPlayer].spawnX;
	this->players[idPlayer].blockY = this->players[idPlayer].spawnY;
	this->players[idPlayer].blockColor = this->players[idPlayer].nextBlockColor;
	this->players[idPlayer].blockShape = this->players[idPlayer].nextBlockShape;
	this->players[idPlayer].blockRotation = 0;
	this->players[idPlayer].blockSpawned = true;
	this->players[idPlayer].nextBlockColor = this->getRandomColor();
	this->players[idPlayer].nextBlockShape = this->getRandomShape();
	this->players[idPlayer].nextSpawn = -1;
	this->players[idPlayer].nextDrop = this->newTime + this->players[idPlayer].dropSpeed;
}

void GameSession::setIntent(int idPlayer, BlockActionIntent intent) {
	this->players[idPlayer].intent = intent;
}

void GameSession::resetIntents() {
	this->players[0].intent = BlockActionIntent::noaction;
	this->players[1].intent = BlockActionIntent::noaction;
}