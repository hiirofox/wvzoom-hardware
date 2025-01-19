#ifndef __USBMIDI_
#define __USBMIDI_

#include "usbd_midi.h"
#include "printf.h"

void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length);


#endif