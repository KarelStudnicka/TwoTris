#pragma once

#include<map>
#include<allegro5/allegro.h>

enum InputKeyType {
	KEY_ESCAPE,
	KEY_ENTER,
	KEY_PAUSE,
	KEY_P1_UP,
	KEY_P1_DOWN,
	KEY_P1_LEFT,
	KEY_P1_RIGHT,
	KEY_P2_UP,
	KEY_P2_DOWN,
	KEY_P2_LEFT,
	KEY_P2_RIGHT,
	KEY_ITEM_LAST
};

enum InputProcessorActionType {
	BASE = 307000,
	KEY_DOWN,
	KEY_UP,
	MOUSE_MOVE
};

struct InputProcessorInputKeyDownData {
	int keycode;
};

struct InputProcessorInputKeyUpData {
	int keycode;
};

struct InputProcessorInputMouseMoveData {
	int x;
	int y;
	int z;
};

union InputProcessorInputData {
	InputProcessorInputKeyDownData keyDown;
	InputProcessorInputKeyUpData KeyUp;
	InputProcessorInputMouseMoveData MouseMove;
};

class InputProcessor
{
public:
	InputProcessor();
	~InputProcessor();

	void init();
	void destroy();
	bool isInputProcessable(ALLEGRO_EVENT *ev);
	void processInput(ALLEGRO_EVENT *ev);
	ALLEGRO_EVENT_SOURCE *getEventSource();
	bool isInputProcessorSource(ALLEGRO_EVENT *ev);
	void freeInput(ALLEGRO_EVENT *ev);

private:
	std::map<int, int> mapInput;
	std::map<int, int> mapInputRev;
	ALLEGRO_EVENT_SOURCE evSource;

	void prepareInputConfig();
	void emitEvent(ALLEGRO_EVENT_TYPE type, InputProcessorInputData * data);
};

extern InputProcessor *inputProcessor;
