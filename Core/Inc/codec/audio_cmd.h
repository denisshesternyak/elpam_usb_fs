#ifndef INC_CODEC_AUDIO_CMD_H_
#define INC_CODEC_AUDIO_CMD_H_

#include "main.h"

void audio_cmd_reset(void);
void audio_cmd_init_power(void);
void audio_cmd_init_playback(void);
void audio_cmd_init_record(void);
void audio_cmd_send_volume(uint8_t lvl);
void audio_cmd_enable_HP(void);
void audio_cmd_enable_LO(void);

void audio_cmd_playback_unmute(void);
void audio_cmd_playback_mute(void);
void audio_cmd_playback_pwr_up();
void audio_cmd_playback_pwr_down();
void audio_cmd_playback_enable(void);
void audio_cmd_playback_disable(void);

void audio_cmd_microphone_unmute(void);
void audio_cmd_microphone_mute(void);
void audio_cmd_pwr_up_microphone(void);
void audio_cmd_microphone_pwr_down(void);
void audio_cmd_microphone_enable(void);
void audio_cmd_microphone_disable(void);

#endif /* INC_CODEC_AUDIO_CMD_H_ */
