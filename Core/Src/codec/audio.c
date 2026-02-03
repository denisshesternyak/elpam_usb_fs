#include <audio.h>
#include "main.h"
#include "math.h"
#include "cmsis_os.h"
#include "queue.h"
#include "string.h"
#include "audio_cmd.h"
#include "audio_regs.h"
#include "audio_generate_sin.h"
//#include "audiofs.h"
#include "lcd_menu.h"
#include "system_status.h"

extern I2S_HandleTypeDef hi2s2;
Audio_Player_t player;
//extern interval_timer_t interval_timer;
extern osMessageQueueId_t xAudioQueueHandle;
extern osMessageQueueId_t xLCDQueueHandle;

static volatile uint32_t start_play;
static uint8_t dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));

// List of acceptable levels
const uint8_t valid_volume_levels[] = {
    80, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122
};

static void audio_start_playback(void);
static void audio_play_playback();
static void audio_stop_playback(void);
static void audio_pause_playback(void);
static void audio_idle(void);

//static void audio_start_record(void);
//static void audio_stop_record(void);

//static bool audio_get_track_name();
//
//static bool audio_get_track_name(void)
//{
//    const char* name = NULL;
//
//    switch (player.current_sin) {
//        case TRACK_1: name = "sine_4~1.wav"; break;
//        case TRACK_2: name = "2.wav"; break;
//        case TRACK_3: name = "3.wav"; break;
//        case TRACK_4: name = "4.wav"; break;
//        default:
//            return false;
//    }
//
//    if (!name) {
//        player.current_filename[0] = '\0';
//        return false;
//    }
//
//    size_t len = sizeof(player.current_filename);
//    strncpy(player.current_filename, name, len - 1);
//    player.current_filename[len - 1] = '\0';
//
//    return true;
//}

void audio_init(void)
{
    memset(&player, 0, sizeof(player));
    player.new_volume = MAX_VOLUME;
//    player.new_volume = DEF_VALUE_VOLUME;

    audio_init_sin_table();

	/*HAL_Delay(10);
	audio_reset();

	HAL_Delay(50);*/

	audio_init_playback();
	//audiofs_init_record();

//	audiofs_init();
}

void audio_process(AudioEvent_t event)
{
	switch(event)
	{
	case AUDIO_IDLE:
		audio_idle();
		break;
	case AUDIO_START:
		audio_start_playback();
		break;
	case AUDIO_PLAY:
		audio_play_playback();
		break;
	case AUDIO_STOP:
		audio_stop_playback();
		break;
	case AUDIO_PAUSE:
		audio_pause_playback();
		break;
	}
}

static void audio_idle(void)
{
	if (player.is_arming)
	{
		player.last_time_arming++;
		player.is_arming = player.last_time_arming < ARMING_TIME;
		if(!player.is_arming) Print_Msg("ARM time's up\r\n");
	}

	if (player.is_announcement)
	{
		player.last_time_announcement++;
		player.is_announcement = player.last_time_announcement < ANNOUNCEMENT_TIME;
		if(!player.is_announcement) Print_Msg("ANNOUNCEMENT time's up\r\n");
	}
}

static void audio_start_playback(void)
{
//	Print_Msg("AUDIO_START\r\n");

	if (player.priority < player.current_priority) return;
	else if (player.priority > player.current_priority && player.is_playing)
	{
		HAL_I2S_DMAStop(&hi2s2);
	}

	player.current_priority = player.priority;

	switch(player.type_input)
	{
	case AUDIO_SIN:
		init_generation(player.current_sin);
		audio_generate_sine(dma_buffer, AUDIO_STEREO_PAIRS_FULL);
		break;
	case AUDIO_SD:
//		if(!audio_get_track_name()) return;
//		if (!audiofs_load_file())
//		{
//			char msg[128];
//			sprintf(msg, "Failure load %s\r\n", player.current_filename);
//			Print_Msg(msg);
//			return;
//		}
		break;
	case AUDIO_IN1:
	case AUDIO_IN2:
	case AUDIO_IN3:
	default:
		break;
	}

	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	HAL_I2S_Init(&hi2s2);
    HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)dma_buffer, AUDIO_HALF_BUFFER_SIZE);

	audio_unmute();
	if(player.new_volume != player.current_volume) audio_set_volume(player.new_volume);

    player.is_playing = true;
    player.is_stoped = false;
    player.is_prepare_stoped = false;
    player.audio_state = AUDIO_PLAY;

	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_PROGRESS, .value = 0 };
    xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
}

static void audio_play_playback()
{
//	Print_Msg("AUDIO_PLAY\r\n");

//	if (!player.is_playing || player.buff_state == BUFFER_IDLE) return;

//	UINT br;

	static uint8_t update_progree = 0;
	if((update_progree++ > COUNT_PROGRESS) || player.is_prepare_stoped)
	{
		update_progree = 0;
		LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_PROGRESS, .value = player.duration };
	    xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
	}

	if(player.is_prepare_stoped)
	{
		player.is_stoped = true;
		return;
	}

	uint32_t offset = (player.buff_state == BUFFER_HALF) ? 0 : AUDIO_HALF_BUFFER_SIZE;
	uint8_t *buf_ptr = dma_buffer + offset;

	switch(player.type_input)
	{
	case AUDIO_SIN:
		audio_generate_sine(buf_ptr, AUDIO_STEREO_PAIRS_HALF);
		break;
	case AUDIO_SD:
//		br = audiofs_read_buffer_part(buf_ptr, AUDIO_HALF_BUFFER_SIZE);
//		if (br <= 0) audio_stop_playback();
		break;
	case AUDIO_IN1:
	case AUDIO_IN2:
	case AUDIO_IN3:
	default:
		break;
	}

	player.buff_state = BUFFER_IDLE;
	player.audio_state = AUDIO_PLAY;
}

static void audio_stop_playback(void)
{
//	Print_Msg("AUDIO_STOP\r\n");

	if (!player.is_playing || player.priority < player.current_priority) return;

	if (player.is_stoped)
	{
		LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .button = BTN_ESC, .action = BA_PRESSED } };
		xQueueSend(xLCDQueueHandle, &lcd_event, portMAX_DELAY);
	}

	player.is_playing = false;
	player.is_stoped = true;
    player.audio_state = AUDIO_IDLE;
	player.buff_state = BUFFER_IDLE;
	player.current_priority = AUDIO_PRIORITY_IDLE;
	player.priority = AUDIO_PRIORITY_IDLE;
	player.duration = 0;
//	player.bytes_read = 0;

    HAL_I2S_DMAStop(&hi2s2);
    hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    HAL_I2S_Init(&hi2s2);

	audio_mute();

//    audiofs_close_file();
}

static void audio_pause_playback(void)
{

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
/*
void audio_start_record(void)
{
	const char *msg = "audio_start_record\r\n";
	Print_Msg(msg);
}

void audio_stop_record(void)
{
	const char *msg = "audio_stop_record\r\n";
	Print_Msg(msg);
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {

    }
}
*/


//uint8_t volume_db_to_bars(int db)
//{
//	if (db <= MIN_VOLUME) return 0;
//	if (db >= MAX_VOLUME) return NUM_VOLUME_BARS - 1;
//	return (uint8_t)((db - MIN_VOLUME) / VOLUME_STEP);
//}
//
//int volume_bars_to_db(uint8_t bar_index)
//{
//	if (bar_index >= NUM_VOLUME_BARS) return MAX_VOLUME;
//	return MIN_VOLUME + bar_index * VOLUME_STEP;
//}
//
//uint8_t find_volume_index(int requested_db)
//{
//    if (requested_db <= MIN_VOLUME) {
//        return 0;
//    }
//    if (requested_db >= MAX_VOLUME) {
//        return NUM_VOLUME_BARS - 1;
//    }
//
//    for (int i = 0; i < NUM_VOLUME_BARS; i++) {
//        if (valid_volume_levels[i] >= requested_db) {
//            return (uint8_t)i;
//        }
//    }
//
//    return NUM_VOLUME_BARS - 1;
//}

static int audio_find_closest_valid_volume(uint8_t target)
{
    if (target <= MIN_VOLUME) return MIN_VOLUME;
    if (target >= MAX_VOLUME) return MAX_VOLUME;

    for (int i = 0; i < NUM_VOLUME_BARS; i++) {
        if (valid_volume_levels[i] >= target) {
            return valid_volume_levels[i];
        }
    }
    return MAX_VOLUME;
}

void audio_set_volume(uint8_t level)
{
	uint8_t corrected_vol = audio_find_closest_valid_volume(level);

    player.current_volume = corrected_vol;

    uint8_t vol;
    if(corrected_vol >= MAX_VOLUME) vol = MAX_VOLUME_CODEC;
    else if(corrected_vol <= MIN_VOLUME) vol = MIN_VOLUME_CODEC;
    else vol = CNVR_VOL(corrected_vol);

    char msg[64];
    sprintf(msg, "Volume set %d\r\n", corrected_vol);
    Print_Msg(msg);

    audio_send_volume(vol);

    system_status.max_volume = (corrected_vol == MAX_VOLUME);

    //uint8_t bar_index = find_volume_index(corrected) + 1;
    //VolumeIndicator_SetLevelSilent(&volumeIndicator, bar_index);
}

