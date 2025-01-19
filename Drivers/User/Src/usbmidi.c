#include "usbmidi.h"


void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length)
{
	int cable,code,message,channel,messageByte1,messageByte2;
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
		uart_printf("\r\nMidi Msg:(%d %d %d) cable:%d code:%d channel:%d",message,messageByte1,messageByte2,
			cable,code,channel);
  }
}
