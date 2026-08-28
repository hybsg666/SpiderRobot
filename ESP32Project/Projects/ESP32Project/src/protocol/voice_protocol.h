#ifndef VOICE_PROTOCOL_H
#define VOICE_PROTOCOL_H

#include <Arduino.h>

// SU-03T 语音帧格式：AA 55 XX 55 AA
#define FRAME_HEADER1  0xAA
#define FRAME_HEADER2  0x55
#define FRAME_FOOTER1  0x55
#define FRAME_FOOTER2  0xAA
#define FRAME_LENGTH   5

#endif
