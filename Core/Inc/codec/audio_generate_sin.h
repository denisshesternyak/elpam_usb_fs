#ifndef INC_CODEC_AUDIO_GENERATE_SIN_H_
#define INC_CODEC_AUDIO_GENERATE_SIN_H_

#include "stdint.h"
#include "main.h"

void audio_init_wave_table(void);
void audio_generate_sine(uint8_t *buffer, uint32_t samples_per_channel);

#endif /* INC_CODEC_AUDIO_GENERATE_SIN_H_ */
