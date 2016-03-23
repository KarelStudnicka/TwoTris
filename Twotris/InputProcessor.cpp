#include <map>
#include <allegro5/allegro.h>
#include "InputProcessor.h"

InputProcessor::InputProcessor() {
}

InputProcessor::~InputProcessor() {
}

void InputProcessor::init() {
	al_install_keyboard();
	this->prepareInputConfig();
	al_init_user_event_source(&this->evSource);
}

void InputProcessor::destroy() {

}

bool InputProcessor::isInputProcessable(ALLEGRO_EVENT *ev) {
	if (
		(ev->type == ALLEGRO_EVENT_KEY_DOWN) || 
		(ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN)
		) return true;
	return false;
}

void InputProcessor::processInput(ALLEGRO_EVENT *ev) {
	if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (this->mapInputRev.count(ev->keyboard.keycode)) {
			InputProcessorInputData *data = new InputProcessorInputData();
			data->keyDown.keycode = this->mapInputRev[ev->keyboard.keycode];
			this->emitEvent(InputProcessorActionType::KEY_DOWN, data);
		}
	}
}

ALLEGRO_EVENT_SOURCE *InputProcessor::getEventSource() {
	return &this->evSource;
}

bool InputProcessor::isInputProcessorSource(ALLEGRO_EVENT *ev) {
	return (ev->any.source == &this->evSource) ? true : false;
}

void InputProcessor::freeInput(ALLEGRO_EVENT *ev) {
	if (ev->user.data1) free((void *)ev->user.data1);
	if (ev->user.data2) free((void *)ev->user.data2);
	if (ev->user.data3) free((void *)ev->user.data3);
	if (ev->user.data4) free((void *)ev->user.data4);
}

void InputProcessor::prepareInputConfig() {
	this->mapInput.clear();
	this->mapInput[InputKeyType::KEY_ESCAPE] = ALLEGRO_KEY_ESCAPE;
	this->mapInput[InputKeyType::KEY_ENTER] = ALLEGRO_KEY_ENTER;

	this->mapInput[InputKeyType::KEY_P1_UP] = ALLEGRO_KEY_UP;
	this->mapInput[InputKeyType::KEY_P1_DOWN] = ALLEGRO_KEY_DOWN;
	this->mapInput[InputKeyType::KEY_P1_LEFT] = ALLEGRO_KEY_LEFT;
	this->mapInput[InputKeyType::KEY_P1_RIGHT] = ALLEGRO_KEY_RIGHT;

	this->mapInput[InputKeyType::KEY_P2_UP] = ALLEGRO_KEY_W;
	this->mapInput[InputKeyType::KEY_P2_DOWN] = ALLEGRO_KEY_S;
	this->mapInput[InputKeyType::KEY_P2_LEFT] = ALLEGRO_KEY_A;
	this->mapInput[InputKeyType::KEY_P2_RIGHT] = ALLEGRO_KEY_D;

	this->mapInputRev.clear();
	if (this->mapInput.size()) for (std::map<int, int>::const_iterator i = this->mapInput.begin(); i != this->mapInput.end(); i++) {
		this->mapInputRev[i->second] = i->first;
	}
}

void InputProcessor::emitEvent(ALLEGRO_EVENT_TYPE type, InputProcessorInputData *data) {
	ALLEGRO_EVENT ev;
	ev.type = type;
	ev.user.data1 = (intptr_t)data;
	ev.user.data2 = NULL;
	ev.user.data3 = NULL;
	ev.user.data4 = NULL;
	al_emit_user_event(&this->evSource, &ev, NULL);
}

InputProcessor *inputProcessor = new InputProcessor();