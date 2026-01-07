#ifndef INC_CODEC_AUDIO_TYPES_H_
#define INC_CODEC_AUDIO_TYPES_H_

#include "fatfs.h"
#include "stdbool.h"

#define AUDIO_BUFFER_SIZE  				16384
#define AUDIO_HALF_BUFFER_SIZE  		8192
#define AUDIO_STEREO_PAIRS_FULL 		4096
#define AUDIO_STEREO_PAIRS_HALF 		2048

#define AUDIO_HEADER_SIZE  				44

typedef enum {
	AUDIO_IDLE,
	AUDIO_START,
	AUDIO_PLAY,
	AUDIO_STOP,
	AUDIO_PAUSE
}AudioState_t;

typedef enum {
	BUFFER_IDLE,
	BUFFER_HALF,
	BUFFER_FULL
}BufferState_t;

typedef enum {
	TRACK_1,
	TRACK_2,
	TRACK_3,
	TRACK_4
}AudioTrack_t;

typedef enum {
	AUDIO_SIN,
	AUDIO_SD,
	AUDIO_IN1,
	AUDIO_IN2,
	AUDIO_IN3,
}AudioTypeOutput_t;

typedef struct
{
	uint32_t ChunkID;
	uint32_t ChunkSize;
	uint32_t Format;
	uint32_t SubChunk1ID;
	uint32_t SubChunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t BlockAlign;
	uint16_t BitPerSample;
	uint32_t SubChunk2ID;
	uint32_t SubChunk2Size;
} WAV_BaseHeader_t;

typedef struct
{
    uint32_t sample_rate;
    uint16_t num_channels;
    uint16_t bits_per_sample;
    uint32_t byte_rate;
    uint32_t block_align;
    uint32_t data_chunk_offset;
    uint32_t data_size;
} WAV_Info_t;

typedef struct
{
    FIL	file;
    uint8_t dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));

    WAV_Info_t wav_info;
    BufferState_t buff_state;
    AudioState_t audio_state;
    AudioTrack_t current_track;
    AudioTypeOutput_t type_output;

    uint32_t file_size;
    uint32_t bytes_read;
    uint32_t current_file_pos;

    uint32_t start_time_arming;

    volatile bool is_playing;
    volatile bool is_paused;
    volatile bool end_of_file;
    volatile bool file_opened;
    volatile bool is_arming;

    char current_filename[128];
} Audio_Player_t;

#endif /* INC_CODEC_AUDIO_TYPES_H_ */
