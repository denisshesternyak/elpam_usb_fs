#include "audiofs.h"
#include <string.h>

#define ROOT_DIR_PATH 		"/"
#define ALARMS_DIR_PATH 	"/alarms"
#define MESSAGES_DIR_PATH 	"/messages"

static void audiofs_list_directory(const TCHAR* path, char list[][_MAX_LFN], uint8_t* size);
static bool audiofs_read_wav_header(AudioFileInfo_t* info);

FATFS fs;
FIL fil;
DIR dir;
FILINFO fno;
FRESULT fresult;
static bool isMounted;

FRESULT audiofs_mount_drive(void)
{
	fresult = f_mount(&fs, "", 1);
    if (fresult == FR_OK) Print_Msg("SD Card mounted successfully\r\n");
    else
    {
        char msg[64];
        sprintf(msg, "ERROR! Mount failed: %d\r\n", fresult);
        Print_Msg(msg);
    }

    isMounted = fresult == FR_OK;
    return fresult;
}

void audiofs_unmount_drive(void)
{
	fresult = f_mount(NULL, "", 1);
	if (fresult == FR_OK) Print_Msg ("SD Card unmounted successfully...\r\n\n\n");
	else Print_Msg("ERROR! in unmounting SD Card\r\n\n\n");
}

static void audiofs_list_directory(const TCHAR* path, char list[][_MAX_LFN], uint8_t* size)
{
	if(!isMounted)
	{
		Print_Msg("SD Card is not mounted!\r\n");
		return;
	}

    char msg[64];
    uint8_t count = 0;

    fresult = f_opendir(&dir, path);
    if (fresult != FR_OK)
    {
        sprintf(msg, "Failed to open %s directory: %d\r\n", path, fresult);
        Print_Msg(msg);
        return;
    }

    for (; count < MAX_MENU_ITEMS; count++)
    {
    	fresult = f_readdir(&dir, &fno);
        if (fresult != FR_OK || fno.fname[0] == 0)
            break;

        if (fno.fattrib & AM_DIR)
        {
        	sprintf(msg, " DIR: %s\r\n", fno.fname);
        }
        else
        {
        	sprintf(msg, " File: %s", fno.fname);
        	if(list)
        	{
        		strncpy(list[count], fno.fname, _MAX_LFN);
            	list[count][_MAX_LFN - 1] = '\0';
        	}
        }
        Print_Msg(msg);
    }

    f_closedir(&dir);

    if(size) *size = count+1;
}

void audiofs_list_root(void)
{
    Print_Msg("\r\nRoot directory contents:\r\n");
    audiofs_list_directory(ROOT_DIR_PATH, NULL, NULL);
}

void audiofs_list_alarms(char list[][_MAX_LFN], uint8_t* size)
{
    Print_Msg("\r\nAlarms directory contents:\r\n");
    audiofs_list_directory(ALARMS_DIR_PATH, list, size);
}

void audiofs_list_messages(char list[][_MAX_LFN], uint8_t* size)
{
    Print_Msg("\r\nMessages directory contents:\r\n");
    audiofs_list_directory(MESSAGES_DIR_PATH, list, size);
}

void audiofs_close_file(AudioFileInfo_t* info)
{
	if(info->isOpened)
	{
		f_close(&fil);
		info->isOpened = false;
	}
}

static bool audiofs_read_wav_header(AudioFileInfo_t* info)
{
    char msg[128];

    fresult = f_read(&fil, &info->header, (UINT)AUDIO_HEADER_SIZE, (UINT *)&info->bytes_read);
    if (fresult != FR_OK || info->bytes_read != AUDIO_HEADER_SIZE)
    {
        sprintf(msg, "Failed to read header of %s (read %lu bytes)\r\n", (char *)info->filename, info->bytes_read);
        Print_Msg(msg);
        f_close(&fil);
        return false;
    }

    sprintf(msg, "\r\n=== %s Header ===\r\n", (char *)info->filename);
    Print_Msg(msg);
    sprintf(msg, " File size     : %lu bytes\r\n", (unsigned long)info->header.ChunkSize);
	Print_Msg(msg);
	sprintf(msg, " Format        : %c%c%c%c\r\n", (char)info->header.Format, (char)(info->header.Format>>8), (char)(info->header.Format>>16), (char)(info->header.Format>>24));
	Print_Msg(msg);
	sprintf(msg, " Sample Rate   : %lu Hz\r\n", (unsigned long)info->header.SampleRate);
	Print_Msg(msg);
	sprintf(msg, " Channels      : %u\r\n", (unsigned)info->header.NumChannels);
	Print_Msg(msg);
	sprintf(msg, " Bits/Sample   : %u\r\n", (unsigned)info->header.BitPerSample);
	Print_Msg(msg);
	sprintf(msg, " ByteRate      : %lu\r\n", (unsigned long)info->header.ByteRate);
	Print_Msg(msg);
	sprintf(msg, " Data ID       : %c%c%c%c\r\n", (char)info->header.SubChunk2ID, (char)(info->header.SubChunk2ID>>8), (char)(info->header.SubChunk2ID>>16), (char)(info->header.SubChunk2ID>>24));
	Print_Msg(msg);
	sprintf(msg, " Data size     : %lu bytes\r\n", (unsigned long)info->header.SubChunk2Size);
	Print_Msg(msg);
	sprintf(msg, " Data size hex : 0x%04lx bytes\r\n", info->header.SubChunk2Size);
	Print_Msg(msg);
//	uint32_t duration = 0;
//	if (info->header.ByteRate > 0)
//	{
//		duration = info->header.ChunkSize / info->header.ByteRate;
//		sprintf(msg, " Duration      : %02lu:%02lu\r\n", duration / 60, duration % 60);
//		Print_Msg(msg);
//	}
	info->position = info->bytes_read;
	return true;
}

bool audiofs_read_file_info(AudioFileInfo_t* info)
{
    fresult = f_open(&fil, info->filename, FA_READ);
    if (fresult != FR_OK) return false;

    info->isOpened = true;
    info->isEnd = false;
    info->file_size = f_size(&fil);

    bool res = audiofs_read_wav_header(info);
    return res;
}

bool audiofs_read_file(AudioFileInfo_t* info, uint8_t *buffer, uint32_t buffer_len)
{
	if(!info->isOpened) return false;

	char msg[100];

//		uint32_t start = HAL_GetTick();
	fresult = f_read(&fil, buffer, (UINT)buffer_len, (UINT *)&info->bytes_read);
//		uint32_t duration = HAL_GetTick() - start;
//		sprintf(msg, "f_read %lu ms\r\n", duration);
//		Print_Msg(msg);
	if (fresult != FR_OK)
	{
		sprintf(msg, "f_read ERROR: %d\r\n", fresult);
		Print_Msg(msg);
		return false;
	}

	if (info->bytes_read < buffer_len)
	{
		info->isEnd = true;
		memset(buffer + info->bytes_read, 0, buffer_len - info->bytes_read);
	}

	info->position += info->bytes_read;

	if(info->position >= info->file_size) info->isEnd = true;

	return true;
}
