#include "midi.h"

#define MidiMsgFifoLen 64

midi_msg MidiMsgFifo[MidiMsgFifoLen];
int midiInPos = 0;
int midiOutPos = 0;

void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length)
{
    int cable, code, message, channel, messageByte1, messageByte2;
    while (usb_rx_buffer_length && *usb_rx_buffer != 0x00)
    {
        cable = usb_rx_buffer[0] >> 4;
        code = usb_rx_buffer[0] & 0x0F;
        message = usb_rx_buffer[1] >> 4;
        channel = usb_rx_buffer[1] & 0x0F;
        messageByte1 = usb_rx_buffer[2];
        messageByte2 = usb_rx_buffer[3];
        usb_rx_buffer += 4;
        usb_rx_buffer_length -= 4;
        // uart_printf("\r\nMidi Msg:(%d %d %d) cable:%d code:%d channel:%d", message, messageByte1, messageByte2, cable, code, channel);

        if (message == 8 || message == 9) // note
        {
            MidiMsgFifo[midiInPos].trig = message - 8;
            MidiMsgFifo[midiInPos].note = messageByte1;
            MidiMsgFifo[midiInPos].freq = 440.0f * powf(2.0f, (float)(messageByte1 - 69) / 12.0f);
            MidiMsgFifo[midiInPos].vol = messageByte2 / 127.0f;
            MidiMsgFifo[midiInPos].channel = channel;
            MidiMsgFifo[midiInPos].chanVal = 0.0f;
            midiInPos = (midiInPos + 1) % MidiMsgFifoLen;
        }
        else if (message == 11) // control change
        {
            MidiMsgFifo[midiInPos].trig = 2;
            MidiMsgFifo[midiInPos].note = 0;
            MidiMsgFifo[midiInPos].freq = 0.0f;
            MidiMsgFifo[midiInPos].vol = 0.0f;
            MidiMsgFifo[midiInPos].channel = channel + messageByte1 * 16;
            MidiMsgFifo[midiInPos].chanVal = messageByte2 / 127.0f;
            midiInPos = (midiInPos + 1) % MidiMsgFifoLen;
        }
    }
}

int HasMidiMsg()
{
    return midiOutPos != midiInPos;
}

midi_msg GetMidiMsg()
{
    midi_msg msg = MidiMsgFifo[midiOutPos];
    midiOutPos = (midiOutPos + 1) % MidiMsgFifoLen;
    return msg;
}
