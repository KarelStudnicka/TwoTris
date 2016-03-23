#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "InputProcessor.h"
#include "Gfx.h"
#include "Game.h"

int targetFPS = 60;
double timePerFrame = 1.0 / targetFPS;

ALLEGRO_TIMER *timerFrame;
ALLEGRO_EVENT_QUEUE *queueMainLoop;

int main(int argc, char **argv) {
	al_init();
	gfx->init();
	gfx->createDisplay();
	inputProcessor->init();
	queueMainLoop = al_create_event_queue();
	timerFrame = al_create_timer(timePerFrame);
	al_register_event_source(queueMainLoop, al_get_timer_event_source(timerFrame));
	al_register_event_source(queueMainLoop, al_get_keyboard_event_source());
	al_register_event_source(queueMainLoop, inputProcessor->getEventSource());
	al_start_timer(timerFrame);

	game = new Game();

	bool doRedraw = false;
	int cnt = 0;
	double tickLast = al_get_time();

	while (!game->shouldQuit()) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(queueMainLoop, &ev);

		if (ev.type == ALLEGRO_EVENT_TIMER) {
			doRedraw = true;
		} else if (inputProcessor->isInputProcessable(&ev)) {
			inputProcessor->processInput(&ev);
		}
		else if (inputProcessor->isInputProcessorSource(&ev)) {
			game->pushInput(&ev);
			inputProcessor->freeInput(&ev);
		}

		if (doRedraw && al_is_event_queue_empty(queueMainLoop)) {
			double tickCurrent = al_get_time();
			double tickDelta = tickCurrent - tickLast;
			tickLast = tickCurrent;
			game->tick(tickDelta);
			doRedraw = false;
			game->render();
		}
		cnt++;
	}

	inputProcessor->destroy();
	gfx->destroyDisplay();
	gfx->destroy();

	delete game;

	return 0;
}