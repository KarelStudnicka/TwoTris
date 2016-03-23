#include <string>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Gfx.h"

Gfx::Gfx() {
	this->targetFPS = 60;
	this->thrDisplayWidth = 700;
	this->thrDisplayHeight = 500;
	this->displayWidth = this->thrDisplayWidth;
	this->displayHeight = this->thrDisplayHeight;

	this->showFPS = true;
	this->fpsActualSecond = -1;
	this->elapsedFrames = 0;
	this->lastFPS = 0;

	this->mutexQueueClosed = al_create_mutex();

	this->queueOpen = this->createQueue(2000);
	this->queueClosed = this->createQueue(2000);
	this->queueRendering = this->createQueue(2000);
}


Gfx::~Gfx() {
	al_destroy_mutex(this->mutexQueueClosed);
	this->destroyQueue(this->queueOpen);
	this->destroyQueue(this->queueOpen);
	this->destroyQueue(this->queueOpen);
}

void Gfx::init() {
	al_init_user_event_source(&this->alEventSource);
	this->alEventQueue = al_create_event_queue();
	al_register_event_source(this->alEventQueue, &this->alEventSource);
	this->gfxThread = al_create_thread(GfxThread, NULL);
	al_start_thread(this->gfxThread);
}

void Gfx::destroy() {
	this->gfxThreadEvent.type = Gfx::EV_FINISH_THREAD;
	al_emit_user_event(&this->alEventSource, &this->gfxThreadEvent, NULL);
	al_destroy_thread(this->gfxThread);
}

RenderPrimitivesQueue *Gfx::createQueue(int limit) {
	RenderPrimitivesQueue *tmp = new RenderPrimitivesQueue();
	tmp->items = new RenderPrimitive[limit];
	tmp->limit = limit;
	tmp->used = 0;
	for (int ti = 0; ti < limit; ti++) {
		tmp->items[ti].actionType = none;
	}
	return tmp;
}

void Gfx::destroyQueue(RenderPrimitivesQueue *queue) {
	delete[] queue->items;
	delete queue;
}

void Gfx::clearQueue(RenderPrimitivesQueue *queue) {
	if (queue->used) {
		for (int ti = 0; ti < queue->used; ti++) {
			queue->items[ti].actionType = none;
		}
		queue->used = 0;
	}
}

void Gfx::createDisplay() {
	this->gfxThreadEvent.type = Gfx::EV_CREATE_DISPLAY;
	al_emit_user_event(&this->alEventSource, &this->gfxThreadEvent, NULL);
}

void Gfx::destroyDisplay() {
	this->gfxThreadEvent.type = Gfx::EV_DESTROY_DISPLAY;
	al_emit_user_event(&this->alEventSource, &this->gfxThreadEvent, NULL);
}

void Gfx::render() {
	this->gfxThreadEvent.type = Gfx::EV_RENDER;
	al_emit_user_event(&this->alEventSource, &this->gfxThreadEvent, NULL);
}

void Gfx::startDrawing() {
	this->clearQueue(this->queueOpen);
}

void Gfx::endDrawing() {
	RenderPrimitivesQueue *queueTemp;

	al_lock_mutex(this->mutexQueueClosed);
	queueTemp = this->queueOpen;
	this->queueOpen = this->queueClosed;
	this->queueClosed = queueTemp;
	al_unlock_mutex(this->mutexQueueClosed);
	queueTemp = NULL;
}

void Gfx::clearBackground(ALLEGRO_COLOR col) {
	int index = this->queueOpen->used;
	this->queueOpen->items[index].actionType = clear;
	this->queueOpen->items[index].data.dataClear.color = col;
	this->queueOpen->used++;
}

void Gfx::putFilledRectangle(int x, int y, int w, int h, ALLEGRO_COLOR colorFill, ALLEGRO_COLOR colorBorder) {
	int index = this->queueOpen->used;
	this->queueOpen->items[index].actionType = putfilledrectangle;
	this->queueOpen->items[index].data.dataPutFilledRectangle.x = x;
	this->queueOpen->items[index].data.dataPutFilledRectangle.y = y;
	this->queueOpen->items[index].data.dataPutFilledRectangle.h = h;
	this->queueOpen->items[index].data.dataPutFilledRectangle.w = w;
	this->queueOpen->items[index].data.dataPutFilledRectangle.colorFill = colorFill;
	this->queueOpen->items[index].data.dataPutFilledRectangle.colorBorder = colorBorder;
	this->queueOpen->used++;
}

void Gfx::putText(char *text, int x, int y, ALLEGRO_COLOR color, int align) {
	int index = this->queueOpen->used;
	this->queueOpen->items[index].actionType = puttext;
	this->queueOpen->items[index].data.dataPutText.text = text;
	this->queueOpen->items[index].data.dataPutText.x = x;
	this->queueOpen->items[index].data.dataPutText.y = y;
	this->queueOpen->items[index].data.dataPutText.align = align;
	this->queueOpen->items[index].data.dataPutText.color = color;
	this->queueOpen->used++;
}

void Gfx::thrCreateDisplay() {
	this->alDisplay = al_create_display(this->thrDisplayWidth, this->thrDisplayHeight);
	al_init_primitives_addon();
	al_init_font_addon();
	this->fontSystem = al_create_builtin_font();
}

void Gfx::thrDestroyDisplay() {
	al_shutdown_primitives_addon();
	al_destroy_display(this->alDisplay);
}

void Gfx::thrRender() {
	RenderPrimitivesQueue *queueTemp;
	
	al_lock_mutex(this->mutexQueueClosed);
	queueTemp = this->queueRendering;
	this->queueRendering = this->queueClosed;
	this->queueClosed = queueTemp;
	al_unlock_mutex(this->mutexQueueClosed);
	queueTemp = NULL;
	if (this->queueRendering->used) {
		for (int itemIndex = 0; itemIndex < this->queueRendering->used; itemIndex++) {
			if (this->queueRendering->items[itemIndex].actionType == clear) {
				al_clear_to_color(this->queueRendering->items[itemIndex].data.dataClear.color);
			} else if (this->queueRendering->items[itemIndex].actionType == putfilledrectangle) {
				al_draw_filled_rectangle(
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.x + 0.5,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.y + 0.5,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.x + this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.w - 1 - 0.5,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.y + this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.h - 1 - 0.5,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.colorFill
					);
				int borderWidth = 2;
				al_draw_rectangle(
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.x + 0.5 + borderWidth,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.y + 0.5 + borderWidth,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.x + this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.w - 1 - 0.5 - +borderWidth,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.y + this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.h - 1 - 0.5 - +borderWidth,
					this->queueRendering->items[itemIndex].data.dataPutFilledRectangle.colorBorder,
					borderWidth * 2
					);
			} else if (this->queueRendering->items[itemIndex].actionType == puttext) {
				al_draw_text(
					this->fontSystem,
					this->queueRendering->items[itemIndex].data.dataPutText.color,
					this->queueRendering->items[itemIndex].data.dataPutText.x,
					this->queueRendering->items[itemIndex].data.dataPutText.y,
					this->queueRendering->items[itemIndex].data.dataPutText.align,
					this->queueRendering->items[itemIndex].data.dataPutText.text
					);
			}
		}
	}

	long currentSec = (long)al_get_time();
	if ((this->fpsActualSecond == -1) || (currentSec > this->fpsActualSecond)) {
		this->fpsActualSecond = currentSec;
		this->lastFPS = this->elapsedFrames;
		this->elapsedFrames = 0;
	}
	this->elapsedFrames++;

	if (this->showFPS) {
		al_draw_text(this->fontSystem, al_map_rgb(0, 0, 0), 10, 10, 0, (std::string("FPS: ") + std::to_string(this->lastFPS)).c_str());
	}

	al_flip_display();
}

Gfx *gfx = new Gfx();

static void *GfxThread(ALLEGRO_THREAD *thr, void *arg) {
	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(gfx->alEventQueue, &ev);
		if (ev.type == Gfx::EV_CREATE_DISPLAY) {
			gfx->thrCreateDisplay();
		} else if (ev.type == Gfx::EV_DESTROY_DISPLAY) {
			gfx->thrDestroyDisplay();
		} else if (ev.type == Gfx::EV_DESTROY_DISPLAY) {
			gfx->thrDestroyDisplay();
		} else if (ev.type == Gfx::EV_RENDER) {
			gfx->thrRender();
		} else if (ev.type == Gfx::EV_FINISH_THREAD) {
			break;
		}
	}

	return 0;
}