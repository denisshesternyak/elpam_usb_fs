#ifndef INC_CODEC_AUDIO_H_
#define INC_CODEC_AUDIO_H_

#include "main.h"
#include "stdbool.h"
#include "audio_types.h"
#include "defines.h"

#define CNVR_VOL(vol) 	((MIN_VOLUME_CODEC + ((vol) - MIN_VOLUME) * 55 / 42))

void audio_init(void);
void audio_process(void);
void audio_set_volume(uint8_t level);

void audio_start_playback(void);
void audio_play_playback();
void audio_stop_playback(void);
void audio_pause_playback(void);

void audio_start_record(void);
void audio_stop_record(void);

#endif /* INC_CODEC_AUDIO_H_ */
