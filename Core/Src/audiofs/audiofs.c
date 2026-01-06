#include "audiofs.h"

static void FS_ListRootDirectory(void);
static void FS_ReadWavHeader(WAV_BaseHeader_t *header);

FATFS fs;
DIR dir;
FILINFO fno;
Audio_Player_t player;

FRESULT FS_MountDrive(void)
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

void FS_UnmountDrive(void)
{
    f_mount(NULL, "", 0);
    Print_Msg("USB Drive unmounted\r\n");
}

static void FS_ListRootDirectory(void)
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
            //FS_ReadWavHeader(fno.fname);
        }
        Print_Msg(msg);
    }

    f_closedir(&dir);
}

void FS_CloseFile(void)
{
	f_close(&player.file);
	player.file_opened = false;
}

static void FS_ReadWavHeader(WAV_BaseHeader_t *header)
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

//void FS_ReadDisk(void)
//{
//    char msg[64];
//    sprintf(msg, "FS_Read started\r\n");
//    Print_Msg(msg);
//
//    if (f_mount(&fs, "", 0) != FR_OK)
//    {
//        if (USB_MountDrive() != FR_OK)
//            return;
//    }
//
//    FS_ListRootDirectory();
//    FS_ReadWavHeader("1.wav");
//}

bool FS_LoadFile(const char* filename)
{
    FRESULT res;
    UINT br;

	FS_ListRootDirectory();

    if (player.file_opened)
    {
        f_close(&player.file);
    }

    memset(&player, 0, sizeof(player));

    res = f_open(&player.file, filename, FA_READ);
    if (res != FR_OK) return false;

    player.file_opened = true;
    player.file_size = f_size(&player.file);
    strncpy(player.current_filename, filename, sizeof(player.current_filename)-1);

    WAV_BaseHeader_t base_hdr;
	FS_ReadWavHeader(&base_hdr);
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

UINT FS_Read_Buffer_Part()
{
	UINT br;
	uint32_t offset;
	uint8_t* buf_ptr;
	char msg[100];

	switch(player.buff_state)
	{
		case BUFFER_HALF: offset = 0; break;
		case BUFFER_FULL: offset = AUDIO_HALF_BUFFER_SIZE; break;
		case BUFFER_IDLE:
		default: return -1;
	}
	buf_ptr = player.dma_buffer + offset;
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
