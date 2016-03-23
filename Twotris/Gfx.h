#pragma once

#include<allegro5/allegro_font.h>

enum RenderPrimitiveType {
	none,
	clear,
	puttext,
	putbitmap,
	putfilledrectangle,
	putrectangle
};

struct RenderPrimitiveDataClear {
	ALLEGRO_COLOR color;
};

struct RenderPrimitiveDataPutText {
	char *text;
	int x, y;
	ALLEGRO_COLOR color;
	int align;
};

struct RenderPrimitiveDataPutBitmap {
	char *text;
	int x, y;
};

struct RenderPrimitiveDataPutFilledRectangle {
	int x, y;
	int w, h;
	ALLEGRO_COLOR colorFill;
	ALLEGRO_COLOR colorBorder;
};

struct RenderPrimitiveDataPutRectangle {
	int x, y;
	int w, h;
	ALLEGRO_COLOR color;
};

union RenderPrimitiveData {
	RenderPrimitiveDataClear dataClear;
	RenderPrimitiveDataPutText dataPutText;
	RenderPrimitiveDataPutBitmap dataPutBitmap;
	RenderPrimitiveDataPutFilledRectangle dataPutFilledRectangle;
	RenderPrimitiveDataPutRectangle dataPutRectangle;
};

struct RenderPrimitive {
	int actionType;
	RenderPrimitiveData data;
};

struct RenderPrimitivesQueue {
	RenderPrimitive *items;
	int limit;
	int used;
};

class Gfx
{
private:
	ALLEGRO_EVENT_SOURCE alEventSource;
	ALLEGRO_THREAD *gfxThread;
	RenderPrimitivesQueue *queueOpen;
	RenderPrimitivesQueue *queueClosed;
	RenderPrimitivesQueue *queueRendering;

	// gfx thread
	int thrDisplayWidth, thrDisplayHeight;
	int targetFPS;
	bool showFPS;
	long fpsActualSecond;
	int elapsedFrames;
	int lastFPS;
	ALLEGRO_FONT *fontSystem;
public:
	static const int EV_BASE = 10000;
	static const int EV_CREATE_DISPLAY = EV_BASE + 1;
	static const int EV_DESTROY_DISPLAY = EV_BASE + 2;
	static const int EV_FINISH_THREAD = EV_BASE + 3;
	static const int EV_RENDER = EV_BASE + 4;

	ALLEGRO_EVENT_QUEUE *alEventQueue;
	ALLEGRO_EVENT gfxThreadEvent;
	ALLEGRO_DISPLAY *alDisplay;
	ALLEGRO_MUTEX *mutexQueueClosed;

	int displayWidth, displayHeight;

	Gfx();
	~Gfx();

	void init();
	void destroy();
	RenderPrimitivesQueue *Gfx::createQueue(int limit);
	void destroyQueue(RenderPrimitivesQueue *queue);
	void clearQueue(RenderPrimitivesQueue *queue);
	void createDisplay();
	void destroyDisplay();
	void render();
	void startDrawing();
	void endDrawing();
	void clearBackground(ALLEGRO_COLOR col);
	void putFilledRectangle(int x, int y, int w, int h, ALLEGRO_COLOR colorFill, ALLEGRO_COLOR colorBorder);
	void putText(char *text, int x, int y, ALLEGRO_COLOR color, int align);
	// gfx thread
	void thrCreateDisplay();
	void thrDestroyDisplay();
	void thrRender();
};

extern Gfx *gfx;

static void *GfxThread(ALLEGRO_THREAD *thr, void *arg);