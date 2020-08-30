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
	uint8_t _repeat = 0;
	bool _state = HIGH;
	bool _longPressed = true;
	uint16_t _duration = 0;
	uint16_t _lastPressed = 0;
	uint16_t _lastReleased = 0;
	bool _busy = false;
	Ticker _timer;

public:
	using onClickCallback = std::function<void(int)>;
	using onClickHandler = CallBackList<onClickCallback>::CallBackHandler;

	using onHoldCallBack = std::function<void(int)>;
    using onHoldHandler  = CallBackList<onHoldCallBack>::CallBackHandler;

    CallBackList<onClickCallback> onClickHandlers;
    CallBackList<onHoldCallBack> onPressHoldHandlers;
	
	onClickHandler onClick(onClickCallback cb) {
		return onClickHandlers.add(cb);
	}
	onHoldHandler onPressHold(onHoldCallBack cb) {
		return onPressHoldHandlers.add(cb);
	}

	Button(uint8_t pin): _pin(pin) {}

	void begin() {
		pinMode(_pin, INPUT_PULLUP);
		_state = digitalRead(_pin);
		_timer.attach_ms_scheduled_accurate(1, [this]() {
			if (_busy) return;
			_busy = true;
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
					if (_lastPressed && !_longPressed && _duration > LONG_PRESS_DURATION) {
						onPressHoldHandlers.execute(_repeat);
						_longPressed = true;
						_repeat = 0;
					}
				} else {
					_duration = millis() - _lastReleased;
					if (_lastReleased && !_longPressed && _repeat && _duration > SHORT_PRESS_DURATION) {
						onClickHandlers.execute(_repeat);
						_repeat = 0;
					}
				}
			}
			_busy = false;
		});
	}
};

#endif
