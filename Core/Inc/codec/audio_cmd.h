#ifndef INC_CODEC_AUDIO_CMD_H_
#define INC_CODEC_AUDIO_CMD_H_

#include "main.h"

void audio_reset(void);
void audio_init_playback(void);
void audio_init_record(void);
void audio_send_volume(uint8_t lvl);

#endif /* INC_CODEC_AUDIO_CMD_H_ */
