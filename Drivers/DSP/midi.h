#pragma once

#include <stdint.h>
#include "midi.h"
#include "printf.h"

typedef struct
{
    // 稍微封装一下音符和参数，便于synth使用

    int trig; // note on/off或非note信息
    int note;
    float freq;
    float vol;

    int channel; // parameter
    float chanVal;
} midi_msg;

void USBD_MIDI_DataInHandler(uint8_t *usb_rx_buffer, uint8_t usb_rx_buffer_length); // 中断别动

int HasMidiMsg(); // 有没有midi消息
midi_msg GetMidiMsg(); // 获取midi消息