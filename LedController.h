#pragma once

#include <stdint.h>
#include "esp32-hal-gpio.h"

////////////////////////////////////////////////////////////////////////////////////
///
/// Class for asynchronously toggling pin
///
////////////////////////////////////////////////////////////////////////////////////
class LedController
{
public:
	LedController() {}
	LedController(uint8_t ledPin, bool pinInverted = false);

	void init(uint8_t ledPin, bool pinInverted = false);

	void on();
	void off();

	void repeatBlink(int timeOn, int timeOff);
	void repeatBlink(int timeOn1, int timeOff1, int timeOn2, int timeOff2);
	void repeatBlink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3, int timeOff3);

	void blink(int timeOn = 100);
	void blink(int timeOn, int timeOff);
	void blink(int timeOn1, int timeOff1, int timeOn2);
	void blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2);
	void blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3);
	void blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3, int timeOff3);

	void process();
	void process(uint32_t time);
	
	void restart();
	void restart(uint32_t time);

private:
	struct SequenceItem
	{
		uint32_t sequence[10]; // off, on, off, on, off, ...
		int sequenceLength;
		int interval;
	};
	
	enum SequenceState
	{
		SequenceOn, SequenceOff, SequenceFinished
	};

	void processItems(uint32_t time);
	bool processItem(const SequenceItem& item, uint32_t time);

	// Returns the state of toggle sequence at time since sequence start
	SequenceState sequenceState(const uint32_t* sequence, int length, uint32_t relativeTime);

private:
	uint8_t pin;
	bool inverted;
	uint32_t sequenceTimeStart;
	SequenceItem repeatItem;
	SequenceItem oneTimeItem;
};

