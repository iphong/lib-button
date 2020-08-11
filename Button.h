//
// Author: Phong Vu
//
#include <Arduino.h>
#include <Ticker.h>
#include "CallBackList.h"

#ifndef __BUTTON_H__
#define __BUTTON_H__

#define MIN_PRESS_DURATION 100
#define SHORT_PRESS_DURATION 300
#define LONG_PRESS_DURATION 600

using namespace experimental::CBListImplentation;

class Button {
protected:
	uint8_t _pin;
	uint8_t _repeat;
	bool _state;
	bool _longPressed = true;
	uint16_t _lastPressed;
	uint16_t _lastReleased;
	uint16_t _duration;
	Ticker _loop;

public:
	using onClickCallback = std::function<void(int)>;
	using onClickHandler = CallBackList<onClickCallback>::CallBackHandler;

	using onHoldCallBack = std::function<void()>;
    using onHoldHandler  = CallBackList<onHoldCallBack>::CallBackHandler;

    CallBackList<onClickCallback> onClickHandlers;
    CallBackList<onHoldCallBack> onHoldHandlers;
	
	Button(uint8_t pin) {
		_pin = pin;
	}
	void onPress(onClickCallback cb) { 
		onClickHandlers.add(cb);
	}
	void onHold(onHoldCallBack cb) {
		onHoldHandlers.add(cb);
	}
	void begin() {
		pinMode(_pin, INPUT_PULLUP);
		_state = digitalRead(_pin);
		_lastReleased = millis();
		_loop.attach_ms_scheduled_accurate(10, [this]() {
			bool state = digitalRead(_pin);
			if (state != _state) {
				if (!state && millis() - _lastReleased > MIN_PRESS_DURATION) {
					_state = state;
					_lastPressed = millis();
				}
				if (state && millis() - _lastPressed > MIN_PRESS_DURATION) {
					_state = state;
					_lastReleased = millis();
					if (_longPressed) {
						_repeat = 0;
					} else {
						_repeat ++;
					}
				}
				_longPressed = false;
			}
			else {
				if (!_state) {
					_duration = millis() - _lastPressed;
					if (!_longPressed && _duration > LONG_PRESS_DURATION) {
						onHoldHandlers.execute();
						_longPressed = true;
						_repeat = 0;
					}
				} else {
					_duration = millis() - _lastReleased;
					if (!_longPressed && _repeat && _duration > SHORT_PRESS_DURATION) {
						onClickHandlers.execute(_repeat);
						_repeat = 0;
					}
				}
			}
		});
	}
};

#endif
