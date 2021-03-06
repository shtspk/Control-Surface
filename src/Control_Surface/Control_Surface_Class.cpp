#include "Control_Surface_Class.h"
#include "./MIDI_Outputs/MIDI_Control_Element.h"
#include "./MIDI_Inputs/MIDI_Input_Element.h"
#include "./Banks/BankSelector.h"
#include "../Helpers/StreamOut.h"

// public:

Control_Surface_ &Control_Surface_::getInstance()
{
    static Control_Surface_ instance;
    return instance;
}
Control_Surface_::~Control_Surface_()
{
    delete new_midi;
}

void Control_Surface_::begin()
{
    if (midi == nullptr) // if this is the first time that begin is executed
    {
        midi = MIDI_Interface::getDefault(); // use the default MIDI interface
        if (midi == nullptr)                 // if it doesn't exist
        {
            new_midi = new USBMIDI_Interface; // create a new USB MIDI interface
            midi = new_midi;
        }
        midi->begin(); // initialize the MIDI interface
    }
}
void Control_Surface_::refresh()
{
    if (midi == nullptr)
        begin(); // make sure that midi != nullptr

    refreshControls();      // refresh all control elements (Analog, AnalogHiRes, Digital, DigitalLatch, RotaryEncoder)
    refreshBankSelectors(); // refresh all bank selectors

#ifdef NO_MIDI_INPUT
    MIDI()->ignoreInput();
#else
    updateMidiInput();
    refreshInputs();
#endif
}

MIDI_Interface *Control_Surface_::MIDI()
{
    return midi;
}

// private:

void Control_Surface_::refreshControls()
{
    for (MIDI_Control_Element *element = MIDI_Control_Element::getFirst(); element != nullptr; element = element->getNext())
        element->refresh();
}

void Control_Surface_::refreshBankSelectors()
{
    for (BankSelector *element = BankSelector::getFirst(); element != nullptr; element = element->getNext())
        element->refresh();
}

#ifndef NO_MIDI_INPUT

void Control_Surface_::updateMidiInput()
{
    MIDI_read_t midiReadResult = midi->read();

    if (midiReadResult == CHANNEL_MESSAGE)
    {
        uint8_t header = midi->ChannelMessage->header;
        uint8_t messageType = header & 0xF0;
        uint8_t channel = header & 0x0F;
        uint8_t data1 = midi->ChannelMessage->data1;
        uint8_t data2 = midi->ChannelMessage->data2;

        if (messageType == CC && data1 == 0x79) // Reset All Controllers
        {
#ifdef DEBUG
            Serial.println("Reset All Controllers");
#endif
            for (MIDI_Input_Element_CC *element = MIDI_Input_Element_CC::getFirst(); element != nullptr; element = element->getNext())
                element->reset();
            for (MIDI_Input_Element_ChannelPressure *element = MIDI_Input_Element_ChannelPressure::getFirst(); element != nullptr; element = element->getNext())
                element->reset();
        }
        else if (messageType == CC && data1 == 0x7B) // All Notes off
        {
#ifdef DEBUG
            DEBUG << "All Notes Off" << endl;
#endif
            for (MIDI_Input_Element_Note *element = MIDI_Input_Element_Note::getFirst(); element != nullptr; element = element->getNext())
                element->reset();
        }
        else
        {
#ifdef DEBUG
            DEBUG << "New midi message:\t" << hex << header << ' ' << data1  << ' ' << data2 << dec << endl;
#endif
            if (messageType == CC) // Control Change
                for (MIDI_Input_Element_CC *element = MIDI_Input_Element_CC::getFirst(); element != nullptr; element = element->getNext())
                {
                    if (element->update(channel, data1))
                        break;
                }
            else if (messageType == NOTE_OFF || messageType == NOTE_ON) // Note
                for (MIDI_Input_Element_Note *element = MIDI_Input_Element_Note::getFirst(); element != nullptr; element = element->getNext())
                {
                    if (element->update(channel, data1))
                        break;
                }
            else if (messageType == CHANNEL_PRESSURE) // Channel Pressure
                for (MIDI_Input_Element_ChannelPressure *element = MIDI_Input_Element_ChannelPressure::getFirst(); element != nullptr; element = element->getNext())
                {
                    if (element->update(channel, data1))
                        break;
                }
        }
    }
    else if (midiReadResult == SYSEX_MESSAGE) // System Exclusive
    {
#ifdef DEBUG
        DEBUG << "System Exclusive:" << tab;
        uint8_t data;
        size_t index = 0;
        do {
            data = midi->SysEx[index++];
            DEBUG << hex << data << ' ' << dec;
        } while (data != SysExEnd);
        DEBUG << endl;
#endif
    }
}
void Control_Surface_::refreshInputs()
{
    for (MIDI_Input_Element_CC *element = MIDI_Input_Element_CC::getFirst(); element != nullptr; element = element->getNext())
        element->refresh();
    for (MIDI_Input_Element_Note *element = MIDI_Input_Element_Note::getFirst(); element != nullptr; element = element->getNext())
        element->refresh();
    for (MIDI_Input_Element_ChannelPressure *element = MIDI_Input_Element_ChannelPressure::getFirst(); element != nullptr; element = element->getNext())
        element->refresh();
}
#endif // ifndef NO_MIDI_INPUT

Control_Surface_ &Control_Surface = Control_Surface_::getInstance();
