//
// Author: Phong Vu
//
#include <Arduino.h>
#include <Ticker.h>

#ifndef __BUTTON_H__
#define __BUTTON_H__

#define MIN_PRESS_DURATION 100
#define SHORT_PRESS_DURATION 300
#define LONG_PRESS_DURATION 600

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
	using callback_t = std::function<void(int)>;

	callback_t onPressCallback;
	callback_t onPressHoldCallback;

	void onPress(callback_t cb) {
		onPressCallback = cb;
	}

	void onPressHold(callback_t cb) {
		onPressHoldCallback = cb;
	}

	Button(uint8_t pin): _pin(pin) {}

	void begin() {
		pinMode(_pin, INPUT_PULLUP);
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
						if (onPressHoldCallback) onPressHoldCallback(_repeat);
						_longPressed = true;
						_repeat = 0;
					}
				} else {
					_duration = millis() - _lastReleased;
					if (_lastReleased && !_longPressed && _repeat && _duration > SHORT_PRESS_DURATION) {
						if (onPressCallback) onPressCallback(_repeat);
						_repeat = 0;
					}
				}
			}
			_busy = false;
		});
	}
};

#endif
