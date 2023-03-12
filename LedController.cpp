#include "LedController.h"

LedController::LedController(uint8_t ledPin, bool pinInverted)
{
	init(ledPin, pinInverted);
}

void LedController::init(uint8_t ledPin, bool pinInverted)
{
	pin = ledPin;
	inverted = pinInverted;

	sequenceTimeStart = 0;
	repeatItem.sequenceLength = 0;
	repeatItem.interval = 0;
	oneTimeItem.sequenceLength = 0;
	oneTimeItem.interval = 0;

	pinMode(pin, OUTPUT);
	inverted ? 	digitalWrite(pin, HIGH) : digitalWrite(pin, LOW);

}

void LedController::on()
{
	oneTimeItem.sequenceLength = 0;
	repeatItem.sequence[0] = 1000;
	repeatItem.sequence[1] = 1000;
	repeatItem.sequenceLength = 2;

	inverted ? 	digitalWrite(pin, LOW) : digitalWrite(pin, HIGH);
}

void LedController::off()
{
	oneTimeItem.sequenceLength = 0;
	repeatItem.sequenceLength = 0;

	inverted ? 	digitalWrite(pin, HIGH) : digitalWrite(pin, LOW);
}

void LedController::repeatBlink(int timeOn, int timeOff)
{
	repeatItem.sequence[0] = timeOn;
	repeatItem.sequence[1] = timeOn + timeOff;
	repeatItem.sequenceLength = 2;
	repeatItem.interval = timeOff;
	restart();
}
	
void LedController::repeatBlink(int timeOn1, int timeOff1, int timeOn2, int timeOff2)
{
	repeatItem.sequence[0] = timeOn1;
	repeatItem.sequence[1] = timeOn1 + timeOff1;
	repeatItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	repeatItem.sequence[3] = timeOn1 + timeOff1 + timeOn2+ timeOff2;
	repeatItem.sequenceLength = 4;
	repeatItem.interval = timeOff2;
	restart();
}

void LedController::repeatBlink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3, int timeOff3)
{
	repeatItem.sequence[0] = timeOn1;
	repeatItem.sequence[1] = timeOn1 + timeOff1;
	repeatItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	repeatItem.sequence[3] = timeOn1 + timeOff1 + timeOn2 + timeOff2;
	repeatItem.sequence[4] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3;
	repeatItem.sequence[5] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3 + timeOff3;
	repeatItem.sequenceLength = 6;
	repeatItem.interval = timeOff3;
	restart();
}

void LedController::blink(int timeOn)
{
	oneTimeItem.sequence[0] = timeOn;
	if (repeatItem.sequenceLength > 0) {
		oneTimeItem.sequence[1] = timeOn + repeatItem.interval;
		oneTimeItem.sequenceLength = 2;
	} else {
		oneTimeItem.sequenceLength = 1;
	}
	restart();
}

void LedController::blink(int timeOn, int timeOff)
{
	oneTimeItem.sequence[0] = timeOn;
	oneTimeItem.sequence[1] = timeOn + timeOff;
	oneTimeItem.sequenceLength = 2;
	restart();
}

void LedController::blink(int timeOn1, int timeOff1, int timeOn2)
{
	oneTimeItem.sequence[0] = timeOn1;
	oneTimeItem.sequence[1] = timeOn1 + timeOff1;
	oneTimeItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	if (repeatItem.sequenceLength > 0) {
		oneTimeItem.sequence[3] = timeOn1 + timeOff1 + timeOn2 + repeatItem.interval;
		oneTimeItem.sequenceLength = 4;
	} else {
		oneTimeItem.sequenceLength = 3;
	}
	restart();
}

void LedController::blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2)
{
	oneTimeItem.sequence[0] = timeOn1;
	oneTimeItem.sequence[1] = timeOn1 + timeOff1;
	oneTimeItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	oneTimeItem.sequence[3] = timeOn1 + timeOff1 + timeOn2 + timeOff2;
	oneTimeItem.sequenceLength = 4;
	restart();
}

void LedController::blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3)
{
	oneTimeItem.sequence[0] = timeOn1;
	oneTimeItem.sequence[1] = timeOn1 + timeOff1;
	oneTimeItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	oneTimeItem.sequence[3] = timeOn1 + timeOff1 + timeOn2 + timeOff2;
	oneTimeItem.sequence[4] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3;
	if (repeatItem.sequenceLength > 0) {
		oneTimeItem.sequence[5] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3 + repeatItem.interval;
		oneTimeItem.sequenceLength = 6;
	} else {
		oneTimeItem.sequenceLength = 5;
	}
	restart();
}

void LedController::blink(int timeOn1, int timeOff1, int timeOn2, int timeOff2, int timeOn3, int timeOff3)
{
	oneTimeItem.sequence[0] = timeOn1;
	oneTimeItem.sequence[1] = timeOn1 + timeOff1;
	oneTimeItem.sequence[2] = timeOn1 + timeOff1 + timeOn2;
	oneTimeItem.sequence[3] = timeOn1 + timeOff1 + timeOn2 + timeOff2;
	oneTimeItem.sequence[4] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3;
	oneTimeItem.sequence[5] = timeOn1 + timeOff1 + timeOn2 + timeOff2 + timeOn3 + timeOff3;
	oneTimeItem.sequenceLength = 6;
	restart();
}

void LedController::process()
{
	processItems(millis());
}

void LedController::process(uint32_t time)
{
	processItems(time);
}

void LedController::restart()
{
	restart(millis());
}

void LedController::restart(uint32_t time)
{
	sequenceTimeStart = time;
	processItems(sequenceTimeStart);
}

void LedController::processItems(uint32_t time)
{
	if (oneTimeItem.sequenceLength > 0) {
		if (processItem(oneTimeItem, time)) {
			oneTimeItem.sequenceLength = 0;
		}
	} else if (repeatItem.sequenceLength > 0) {
		processItem(repeatItem, time);
	}
}

bool LedController::processItem(const SequenceItem& item, uint32_t time)
{
	// Get sequence state
	uint32_t timeInSequence = time - sequenceTimeStart;
	SequenceState state = sequenceState(item.sequence, item.sequenceLength, timeInSequence);

	if (inverted) {
		if (state == SequenceOn)
			digitalWrite(pin, LOW);
		else
			digitalWrite(pin, HIGH);
	}
	else {
		if (state == SequenceOn)
			digitalWrite(pin, HIGH);
		else
			digitalWrite(pin, LOW);
	}

	// Reset sequence
	if (state == SequenceFinished) {
		sequenceTimeStart = time;
		return true;
	}
	return false;
}

LedController::SequenceState LedController::sequenceState(const uint32_t* sequence, int length, uint32_t relativeTime)
{
	if (length == 0)
		return SequenceFinished;
	if (relativeTime >= sequence[length - 1])
		return SequenceFinished;

	//      off     on     off
	//      100     300    500
	// 0   - on
	// 100 - off
	// 200 - off
	// 300 - on
	// 400 - on
	// 500 - end

	bool state = true;
	for (int i = 0; i < length; i++) {
		if (sequence[i] > relativeTime)
			break;
		state = !state;
	}

	if (state)
		return SequenceOn;
	return SequenceOff;
}
