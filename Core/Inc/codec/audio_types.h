#ifndef INC_CODEC_AUDIO_TYPES_H_
#define INC_CODEC_AUDIO_TYPES_H_

#include "fatfs.h"
#include "stdbool.h"

#define AUDIO_BUFFER_SIZE  				16384
#define AUDIO_HALF_BUFFER_SIZE  		8192
#define AUDIO_HEADER_SIZE  				44

typedef enum {
	BUFFER_IDLE,
	BUFFER_HALF,
	BUFFER_FULL
}BufferState;

typedef enum {
	TRACK_1,
	TRACK_2,
	TRACK_3,
	TRACK_4
}AudioTrack;

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
    uint8_t  dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));

    WAV_Info_t wav_info;
    BufferState buff_state;

    uint32_t file_size;
    uint32_t bytes_read;
    uint32_t current_file_pos;

    volatile bool is_playing;
    volatile bool is_paused;
    volatile bool end_of_file;
    volatile bool file_opened;

    char current_filename[128];
} Audio_Player_t;

#endif /* INC_CODEC_AUDIO_TYPES_H_ */
