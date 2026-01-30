#ifndef INC_CODEC_AUDIO_H_
#define INC_CODEC_AUDIO_H_

#include "main.h"
#include "stdbool.h"
#include "audio_types.h"
#include "defines.h"

#define ARMING_TIME  			10U
#define ANNOUNCEMENT_TIME  		180U
#define COUNT_PROGRESS  		10U

#define CNVR_VOL(vol) 			((MIN_VOLUME_CODEC + ((vol) - MIN_VOLUME) * 55 / 42))

void audio_init(void);
void audio_process(AudioEvent_t event);
void audio_set_volume(uint8_t level);

#endif /* INC_CODEC_AUDIO_H_ */
