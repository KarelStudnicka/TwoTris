#pragma once

#include <vector>
#include <allegro5/allegro.h>

enum BlockActionIntent {
	noaction,
	moveleft,
	moveright,
	movedown,
	rotatecw
};

struct PlayerState {
	int spawnX;
	int spawnY;
	float dropSpeed; // one row drop every dropSpeed second
	float nextDrop; // block will fall one row in game time nextDrop
	double nextSpawn; // next block will spawn in nextSpawn GameSession time, unles set to -1.0
	int nextBlockShape;
	int nextBlockColor;
	bool blockSpawned;
	int blockX;
	int blockY;
	int blockRotation;
	int blockShape;
	int blockColor;
	int score;
	BlockActionIntent intent;
	int intentBlockX;
	int intentBlockY;
	int intentBlockRotation;
};

class GameSession
{
public:
	const int GRID_WIDTH = 10;
	const int GRID_HEIGHT = 20;
	const int BLOCK_WIDTH = 19;
	const int BLOCK_HEIGHT = 19;
	const int BLOCK_SPACING = 1;

	typedef std::vector<std::vector<int>> Grid;
	typedef std::vector<ALLEGRO_COLOR> BlockColors;
	typedef std::vector<int> Shape;
	struct ShapeStruct {
		Shape shape;
		int size;
	};
	typedef std::vector<ShapeStruct> Shapes;
	typedef std::vector<int> Rotation;
	typedef std::vector<Rotation> Rotations;

	GameSession();
	~GameSession();

	void reset(int playerCount);
	void pushInput(ALLEGRO_EVENT *ev);
	void tick(double timePassed); // since last tick() call timePassed seconds passed
	void removeFullLines(); // check for full lines in grid and removes them
	void removeLine(int y); // remove line y
	bool collideWithGrid(int idPlayer); // does the intent collide with grid / border?
	bool shouldDrop(int idPlayer); // should block fall one row in current time (newTime)?
	bool canDrop(int idPlayer); // could the block drop one row?
	void drop(int idPlayer); // drop block by one row
	void assimilate(int idPlayer); // assimilate block into grid
	void render();

private:
	Grid grid;
	BlockColors blockColors;
	Shapes shapes;
	Rotations rotations3;
	Rotations rotations4;
	double time;
	double newTime;
	std::vector<PlayerState> players;
	int playerCount;

	void renderGrid(int x, int y);
	void renderShape(int baseX, int baseY, int x, int y, int idShape, int idRotation, int idColor);
	void prepareShapes();
	void prepareRotations();
	int getRandomColor();
	int getRandomShape();
	void resetPlayer(int idPlayer);
	void resetSpawners(int playerCount);
	bool shouldSpawnBlock(int idPlayer);
	void spawnBlock(int idPlayer);
	void setIntent(int idPlayer, BlockActionIntent intent);
	void resetIntents();
};