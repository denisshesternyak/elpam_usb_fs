#include <audio.h>
#include "main.h"
#include "math.h"
#include "cmsis_os.h"
#include "queue.h"
#include "string.h"
#include "audio_cmd.h"
#include "audio_regs.h"
#include "audio_generate_sin.h"
#include "audiofs.h"
#include "lcd_menu.h"
#include "system_status.h"

extern I2S_HandleTypeDef hi2s2;
Audio_Player_t player;
//extern interval_timer_t interval_timer;
extern osMessageQueueId_t xAudioQueueHandle;
extern osMessageQueueId_t xLCDQueueHandle;

static uint8_t dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));

// List of acceptable levels
const uint8_t valid_volume_levels[] = {
    80, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122
};

// PROCESS
static void audio_start(void);
static void audio_play(void);
static void audio_stop(void);
static void audio_pause(void);
static void audio_timer(void);
static void audio_volume(void);

// SINUS
static void audio_start_sinus(void);
static void audio_play_sinus(void);
static void audio_stop_sinus(void);

// SD
static void audio_start_sd(void);
static void audio_play_sd(void);
static void audio_stop_sd(void);

// MIC
static void audio_start_mic(void);
static void audio_play_mic(void);
static void audio_stop_mic(void);

// MOTOROLA
static void audio_start_motorola(void);
static void audio_play_motorola(void);
static void audio_stop_motorola(void);

// DTMF
static void audio_start_dtmf(void);
static void audio_play_dtmf(void);
static void audio_stop_dtmf(void);

static void start_playback(void);
static void stop_playback(void);
static void check_progress(void);

void audio_init(void)
{
    memset(&player, 0, sizeof(player));
    memcpy(player.valid_volume_levels, valid_volume_levels, NUM_VALID_LEVELS);

    player.volume_level = 15;
    if(player.volume_level < 1) player.volume_level = 1;
    player.volume = player.valid_volume_levels[player.volume_level-1];
//    player.volume = DEF_VALUE_VOLUME;

    audio_init_sin_table();

    osDelay(10);
	audio_cmd_reset();
	osDelay(50);

	audio_cmd_init_power();
	audio_cmd_init_playback();
	audio_cmd_init_record();
	//audiofs_init_record();

	audiofs_mount_drive();
	audiofs_list_root_directory();
}

void audio_process(AudioEvent_t event)
{
	switch(event)
	{
	case AUDIO_START: audio_start(); break;
	case AUDIO_PLAY: audio_play(); break;
	case AUDIO_STOP: audio_stop(); break;
	case AUDIO_PAUSE: audio_pause(); break;
	case AUDIO_TIMER: audio_timer(); break;
	case AUDIO_VOLUME: audio_volume(); break;
	default: break;
	}
}

static void audio_start(void)
{
//	Print_Msg("AUDIO_START\r\n");

	if (player.priority < player.current_priority) return;

	player.current_priority = player.priority;

	switch(player.type_input)
	{
	case AUDIO_SIN: audio_start_sinus(); break;
	case AUDIO_SD: audio_start_sd(); break;
	case AUDIO_MIC: audio_start_mic(); break;
	case AUDIO_MOTOROLA: audio_start_motorola(); break;
	case AUDIO_DTMF: audio_start_dtmf(); break;
	default: break;
	}
}

static void audio_play()
{
//	Print_Msg("AUDIO_PLAY\r\n");

	switch(player.type_input)
	{
	case AUDIO_SIN: audio_play_sinus(); break;
	case AUDIO_SD: audio_play_sd(); break;
	case AUDIO_MIC: audio_play_mic(); break;
	case AUDIO_MOTOROLA: audio_play_motorola(); break;
	case AUDIO_DTMF: audio_play_dtmf(); break;
	default: break;
	}
}

static void audio_stop(void)
{
//	Print_Msg("AUDIO_STOP\r\n");

	if (player.priority < player.current_priority) return;

	switch(player.type_input)
	{
	case AUDIO_SIN: audio_stop_sinus(); break;
	case AUDIO_SD: audio_stop_sd(); break;
	case AUDIO_MIC: audio_stop_mic(); break;
	case AUDIO_MOTOROLA: audio_stop_motorola(); break;
	case AUDIO_DTMF: audio_stop_dtmf(); break;
	default: break;
	}
}

static void audio_pause(void)
{

}

static void audio_timer(void)
{
	if (player.is_arming)
	{
		player.last_time_arming++;
		player.is_arming = player.last_time_arming < ARMING_TIME;
		if(!player.is_arming)
		{
			player.last_time_arming = 0;
			Print_Msg("ARM time's up\r\n");
		}
	}

	if (player.is_announcement)
	{
		player.last_time_announcement++;
		player.is_announcement = player.last_time_announcement < ANNOUNCEMENT_TIME;
		if(!player.is_announcement)
		{
			player.last_time_announcement = 0;
			Print_Msg("ANNOUNCEMENT time's up\r\n");
			LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .button = BTN_ESC, .action = BA_PRESSED } };
			xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
		}
	}
}

static void audio_volume(void)
{
	if(!player.is_playing)
	{
//			char msg[64];
//			sprintf(msg, "*--lvl %d, vol %d\r\n", player.volume_level, player.volume );
//			Print_Msg(msg);
		audio_set_volume(player.volume);
	}
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance != SPI2)  return;
	if (!player.is_playing || player.audio_state == AUDIO_STOP) return;

	player.buff_state = BUFFER_HALF;
	if(player.is_stoped)
		player.audio_state = AUDIO_STOP;

	xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance != SPI2)  return;
	if (!player.is_playing || player.audio_state == AUDIO_STOP) return;

	player.buff_state = BUFFER_FULL;
	if(player.is_stoped)
		player.audio_state = AUDIO_STOP;

	xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
}

static int audio_find_closest_valid_volume(uint8_t target)
{
    if (target <= MIN_VOLUME) return MIN_VOLUME;
    if (target >= MAX_VOLUME) return MAX_VOLUME;

    for (int i = 0; i < NUM_VOLUME_BARS; i++) {
        if (player.valid_volume_levels[i] >= target) {
        	player.volume_level = i+1;
//        	char msg[64];
//        	sprintf(msg, "*lvl %d\r\n", player.volume_level );
//        	Print_Msg(msg);
            return player.valid_volume_levels[i];
        }
    }
    return MAX_VOLUME;
}

void audio_set_volume(uint8_t level)
{
	uint8_t corrected_vol = audio_find_closest_valid_volume(level);

	player.volume = corrected_vol;

//	char msg[64];
//	sprintf(msg, "*vol %d\r\n", player.volume );
//	Print_Msg(msg);

    uint8_t vol;
    if(corrected_vol >= MAX_VOLUME) vol = MAX_VOLUME_CODEC;
    else if(corrected_vol <= MIN_VOLUME) vol = MIN_VOLUME_CODEC;
    else vol = CNVR_VOL(corrected_vol);

//    char msg[64];
//    sprintf(msg, "Volume set %d\r\n", corrected_vol);
//    Print_Msg(msg);

    audio_cmd_send_volume(vol);

    system_status.max_volume = (corrected_vol == MAX_VOLUME);

    //uint8_t bar_index = find_volume_index(corrected) + 1;
    //VolumeIndicator_SetLevelSilent(&volumeIndicator, bar_index);
}

// SINUS
static void audio_start_sinus(void)
{
	init_generation(player.current_sin);
	audio_generate_sine(dma_buffer, AUDIO_STEREO_PAIRS_FULL);

	start_playback();
}

static void audio_play_sinus(void)
{
	check_progress();

	if(player.is_prepare_stoped)
	{
		player.is_stoped = true;
		return;
	}

	uint32_t offset = (player.buff_state == BUFFER_HALF) ? 0 : AUDIO_HALF_BUFFER_SIZE;
	uint8_t *buf_ptr = dma_buffer + offset;

	audio_generate_sine(buf_ptr, AUDIO_STEREO_PAIRS_HALF);

	player.buff_state = BUFFER_IDLE;
	player.audio_state = AUDIO_PLAY;
}

static void audio_stop_sinus(void)
{
	stop_playback();
}

// SD
static void audio_start_sd(void)
{
	if (!player.file_info.filename) return;
	if (!audiofs_read_file_info(&player.file_info))
	{
		char msg[128];
		sprintf(msg, "Failure load %s\r\n", player.file_info.filename);
		Print_Msg(msg);
		return;
	}

	start_playback();
}

static void audio_play_sd(void)
{
	check_progress();

	uint32_t offset = (player.buff_state == BUFFER_HALF) ? 0 : AUDIO_HALF_BUFFER_SIZE;
	uint8_t *buf_ptr = dma_buffer + offset;

	bool res = audiofs_read_file(&player.file_info, buf_ptr, AUDIO_HALF_BUFFER_SIZE);
	if (player.file_info.isEnd || !res) audio_stop();

	player.buff_state = BUFFER_IDLE;
	player.audio_state = AUDIO_PLAY;
}

static void audio_stop_sd(void)
{
    stop_playback();

    audiofs_close_file(&player.file_info);
    memset(&player.file_info, 0, sizeof(player.file_info));
}

// MIC
static void audio_start_mic(void)
{
//		audio_cmd_microphone_enable();
}

static void audio_play_mic(void)
{

}

static void audio_stop_mic(void)
{
//	audio_cmd_microphone_disable();
}

// MOTOROLA
static void audio_start_motorola(void)
{
	player.is_motorola = true;

	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .action = BA_PRESSED } };
	lcd_event.btn.button = BTN_MOTOROLA;
	xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
}

static void audio_play_motorola(void)
{

}

static void audio_stop_motorola(void)
{
	player.is_motorola = false;

	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .action = BA_PRESSED } };
	lcd_event.btn.button = BTN_ESC;
	xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
}

// DTMF
static void audio_start_dtmf(void)
{

}

static void audio_play_dtmf(void)
{

}

static void audio_stop_dtmf(void)
{

}


static void start_playback(void)
{
	if (player.priority > player.current_priority && player.is_playing)
	{
		HAL_I2S_DMAStop(&hi2s2);
	}

	audio_cmd_playback_enable();

	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	HAL_I2S_Init(&hi2s2);
	HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)dma_buffer, AUDIO_HALF_BUFFER_SIZE);

//	audio_set_volume(player.volume);

	player.is_playing = true;
	player.is_stoped = false;
	player.is_prepare_stoped = false;
	player.audio_state = AUDIO_PLAY;
}

static void stop_playback(void)
{
	audio_cmd_playback_disable();

	if (!player.is_playing) return;

	if (player.is_stoped)
	{
		LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .button = BTN_ESC, .action = BA_PRESSED } };
		xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
	}

	player.is_playing = false;
	player.is_stoped = true;
	player.audio_state = AUDIO_TIMER;
	player.buff_state = BUFFER_IDLE;
	player.duration = 0;

	HAL_I2S_DMAStop(&hi2s2);
	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	HAL_I2S_Init(&hi2s2);

	player.current_priority = AUDIO_PRIORITY_IDLE;
	player.priority = AUDIO_PRIORITY_IDLE;
}

static void check_progress(void)
{
	static uint8_t update_progree = 0;
	if((update_progree++ > COUNT_PROGRESS) || player.is_prepare_stoped)
	{
		update_progree = 0;
		LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_PROGRESS, .value = player.duration };
		xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
	}
}
