#include "ShiftRegisterOut.h"
#include "ExtendedInputOutput.h"

using namespace ExtIO;

ShiftRegisterOut::ShiftRegisterOut(pin_t dataPin, pin_t clockPin, pin_t latchPin, uint8_t bitOrder, pin_t length)
    : dataPin(dataPin), clockPin(clockPin), latchPin(latchPin), bitOrder(bitOrder)
{
    this->length = length;
    bufferLength = (length + 7) / 8;
    stateBuffer = (uint8_t *)malloc(bufferLength);
    ExtendedIO.add(this);
}
ShiftRegisterOut::~ShiftRegisterOut()
{
    free(stateBuffer);
}

void ShiftRegisterOut::digitalWrite(pin_t pin, uint8_t val)
{
#ifdef DEBUG
    Serial.print("digitalWrite(");
    Serial.print(pin);
    Serial.print(", ");
    Serial.print(val);
    Serial.print(");\r\n");
#endif

    int8_t bufferIndex = pinToBufferIndex(pin);
#ifdef DEBUG_F
    Serial.print("bufferIndex: ");
    Serial.println(bufferIndex);
    Serial.print("pinToBitMask: ");
    Serial.println(pinToBitMask(pin));
#endif

    if (bufferIndex == INVALID_PIN)
        return;
    if (val)
        stateBuffer[bufferIndex] |= pinToBitMask(pin);
    else
        stateBuffer[bufferIndex] &= ~pinToBitMask(pin);
    refresh();
}

void ShiftRegisterOut::pinMode(pin_t pin, uint8_t mode)
{
    //#ifdef DEBUG
    Serial.print("ShiftRegPinMode(");
    Serial.print(pin);
    Serial.print(", ");
    Serial.print(mode);
    Serial.print(");\r\n");
    //#endif
}

void ShiftRegisterOut::begin()
{
#ifdef DEBUG
    Serial.println("ShiftOut\tBegin");
#endif
    ExtIO::pinMode(dataPin, OUTPUT);
    ExtIO::pinMode(clockPin, OUTPUT);
    ExtIO::pinMode(latchPin, OUTPUT);
    reset();
#ifdef DEBUG_F
    Serial.print("length: ");
    Serial.println(length);
    Serial.print("bufferLength: ");
    Serial.println(bufferLength);
#endif
}

void ShiftRegisterOut::refresh()
{
#ifdef DEBUG
    Serial.println("ShiftOut\tRefresh");
#endif
    ExtIO::digitalWrite(latchPin, LOW);
    if (bitOrder == LSBFIRST)
        for (uint8_t i = 0; i < bufferLength; i++)
            ExtIO::shiftOut(dataPin, clockPin, LSBFIRST, stateBuffer[i]);
    else
        for (int8_t i = bufferLength - 1; i >= 0; i--)
            ExtIO::shiftOut(dataPin, clockPin, MSBFIRST, stateBuffer[i]);
    ExtIO::digitalWrite(latchPin, HIGH);
}

void ShiftRegisterOut::reset()
{
#ifdef DEBUG
    Serial.println("ShiftOut\tReset");
#endif
    memset(stateBuffer, 0, bufferLength);
    refresh();
}

pin_t ShiftRegisterOut::green(pin_t id)
{
    return 3 * id + greenBit + offset;
}
pin_t ShiftRegisterOut::red(pin_t id)
{
    return 3 * id + redBit + offset;
}
pin_t ShiftRegisterOut::blue(pin_t id)
{
    return 3 * id + blueBit + offset;
}