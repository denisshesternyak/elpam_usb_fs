#ifndef INC_CODEC_AUDIO_H_
#define INC_CODEC_AUDIO_H_

#include "main.h"
#include "stdbool.h"
#include "audio_types.h"

void audio_init(void);
void audio_start_playback(const char *filename);
void audio_stop_playback(void);

void audio_start_record(void);
void audio_stop_record(void);
void audio_process(void);

#endif /* INC_CODEC_AUDIO_H_ */
