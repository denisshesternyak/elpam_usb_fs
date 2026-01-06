#include "audiofs.h"
#include "audio_types.h"

static FRESULT audiofs_mount_drive(void);
static void audiofs_unmount_drive(void);
static void audiofs_list_root_directory(void);
static void audiofs_read_wav_header(WAV_BaseHeader_t *header);

FATFS fs;
DIR dir;
FILINFO fno;
extern Audio_Player_t player;

static FRESULT audiofs_mount_drive(void)
{
    FRESULT res = f_mount(&USBHFatFS, "", 1);
    if (res == FR_OK)
    {
        Print_Msg("USB Drive mounted successfully\r\n");
    }
    else
    {
        char msg[64];
        sprintf(msg, "Mount failed: %d\r\n", res);
        Print_Msg(msg);
    }
    return res;
}

static void audiofs_unmount_drive(void)
{
    f_mount(NULL, "", 0);
    Print_Msg("USB Drive unmounted\r\n");
}

static void audiofs_list_root_directory(void)
{
    FRESULT res;
    char msg[80];

    sprintf(msg, "\r\nRoot directory contents:\r\n");
    Print_Msg(msg);

    res = f_opendir(&dir, "/");
    if (res != FR_OK)
    {
        sprintf(msg, "Failed to open root directory: %d\r\n", res);
        Print_Msg(msg);
        return;
    }

    for (;;)
    {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
            break;

        if (fno.fattrib & AM_DIR)
        {
            sprintf(msg, "  <DIR>  %s\r\n", fno.fname);
        }
        else
        {
        	sprintf(msg, "%s\r\n", fno.fname);
            //audiofs_read_wav_header(fno.fname);
        }
        Print_Msg(msg);
    }

    f_closedir(&dir);
}

void audiofs_init(void)
{
	audiofs_mount_drive();
	audiofs_list_root_directory();
}

void audiofs_close_file(void)
{
	if(player.file_opened)
	{
		f_close(&player.file);
		player.file_opened = false;
	}
}

static void audiofs_read_wav_header(WAV_BaseHeader_t *header)
{
    FRESULT res;
    UINT br;
    char msg[200];

    res = f_read(&player.file, header, AUDIO_HEADER_SIZE, &br);
    if (res != FR_OK || br != 44)
    {
        sprintf(msg, "Failed to read header of %s (read %u bytes)\r\n", (char *)player.current_filename, br);
        Print_Msg(msg);
        f_close(&player.file);
        return;
    }

    sprintf(msg, "\r\n=== %s Header ===\r\n", (char *)player.current_filename);
    Print_Msg(msg);
    sprintf(msg, " File size     : %lu bytes\r\n", (unsigned long)header->ChunkSize);
	Print_Msg(msg);
	sprintf(msg, " Format        : %c%c%c%c\r\n", (char)header->Format, (char)(header->Format>>8), (char)(header->Format>>16), (char)(header->Format>>24));
	Print_Msg(msg);
	sprintf(msg, " Sample Rate   : %lu Hz\r\n", (unsigned long)header->SampleRate);
	Print_Msg(msg);
	sprintf(msg, " Channels      : %u\r\n", (unsigned)header->NumChannels);
	Print_Msg(msg);
	sprintf(msg, " Bits/Sample   : %u\r\n", (unsigned)header->BitPerSample);
	Print_Msg(msg);
	sprintf(msg, " ByteRate      : %lu\r\n", (unsigned long)header->ByteRate);
	Print_Msg(msg);
	sprintf(msg, " Data ID       : %c%c%c%c\r\n", (char)header->SubChunk2ID, (char)(header->SubChunk2ID>>8), (char)(header->SubChunk2ID>>16), (char)(header->SubChunk2ID>>24));
	Print_Msg(msg);
	sprintf(msg, " Data size     : %lu bytes\r\n", (unsigned long)header->SubChunk2Size);
	Print_Msg(msg);
	sprintf(msg, " Data size hex : 0x%04lx bytes\r\n", header->SubChunk2Size);
	Print_Msg(msg);
	uint32_t duration = 0;
	if (header->ByteRate > 0)
	{
		duration = header->ChunkSize / header->ByteRate;
		sprintf(msg, " Duration      : %02lu:%02lu\r\n", duration / 60, duration % 60);
		Print_Msg(msg);
	}
}

bool audiofs_load_file(const char* filename)
{
    FRESULT res;
    UINT br;

    res = f_open(&player.file, filename, FA_READ);
    if (res != FR_OK) return false;

    player.file_opened = true;
    player.file_size = f_size(&player.file);
    strncpy(player.current_filename, filename, sizeof(player.current_filename)-1);

    WAV_BaseHeader_t base_hdr;
    audiofs_read_wav_header(&base_hdr);
    player.bytes_read += AUDIO_HEADER_SIZE;
    res = f_read(&player.file, player.dma_buffer, AUDIO_BUFFER_SIZE, &br);
	if (res != FR_OK)
	{
		f_close(&player.file);
		player.file_opened = false;
		return false;
	}
	player.bytes_read += br;

	if (br < AUDIO_HALF_BUFFER_SIZE) {
		memset(player.dma_buffer + br, 0, AUDIO_HALF_BUFFER_SIZE - br);
	}

    player.wav_info.sample_rate     = base_hdr.SampleRate;
    player.wav_info.num_channels    = base_hdr.NumChannels;
    player.wav_info.bits_per_sample = base_hdr.BitPerSample;
    player.wav_info.byte_rate       = base_hdr.ByteRate;
    player.wav_info.block_align     = base_hdr.BlockAlign;
    player.wav_info.data_chunk_offset = AUDIO_HEADER_SIZE;
    player.wav_info.data_size       = base_hdr.SubChunk2Size;
    player.buff_state = BUFFER_IDLE;

    return true;
}

UINT audiofs_read_buffer_part()
{
	UINT br;
	uint8_t* buf_ptr;
	char msg[100];

	buf_ptr = player.dma_buffer + (player.buff_state == BUFFER_HALF) ? 0 : AUDIO_HALF_BUFFER_SIZE;
///////////
		uint32_t start = HAL_GetTick();
	FRESULT res = f_read(&player.file, buf_ptr, AUDIO_HALF_BUFFER_SIZE, &br);
		uint32_t duration = HAL_GetTick() - start;
		sprintf(msg, "f_read %lu ms\r\n", duration);
		Print_Msg(msg);
////////////////
	if (res != FR_OK)
	{
		sprintf(msg, "f_read ERROR: %d\r\n", res);
		Print_Msg(msg);
		return -1;
	}

	if (br < AUDIO_HALF_BUFFER_SIZE)
	{
		memset(buf_ptr + br, 0, AUDIO_HALF_BUFFER_SIZE - br);
	}
	player.buff_state = BUFFER_IDLE;
	player.bytes_read += br;
	return br;
}
