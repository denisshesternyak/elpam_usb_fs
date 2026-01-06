#ifndef INC_CODEC_AUDIO_CMD_H_
#define INC_CODEC_AUDIO_CMD_H_

#include "main.h"

void audio_write_cmd(uint8_t reg, uint8_t value);
uint8_t audio_read_cmd(uint8_t reg);

#endif /* INC_CODEC_AUDIO_CMD_H_ */
