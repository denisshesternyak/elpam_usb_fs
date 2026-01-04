#include "audiofs.h"
#include "fatfs.h"

typedef struct
{
  uint32_t   ChunkID;       /* 0 */
  uint32_t   ChunkSize;     /* 4 */
  uint32_t   Format;    	/* 8 */
  uint32_t   SubChunk1ID;   /* 12 */
  uint32_t   SubChunk1Size; /* 16 */
  uint16_t   AudioFormat;   /* 20 */
  uint16_t   NumChannels;   /* 22 */
  uint32_t   SampleRate;    /* 24 */
  uint32_t   ByteRate;      /* 28 */
  uint16_t   BlockAlign;    /* 32 */
  uint16_t   BitPerSample;  /* 34 */
  uint32_t   SubChunk2ID;   /* 36 */
  uint32_t   SubChunk2Size; /* 40 */
}WAV_FormatTypeDef;

FATFS fs;
DIR dir;
FILINFO fno;
FIL file;

void USB_ReadWavHeader(const char *filename);

FRESULT USB_MountDrive(void)
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

void USB_UnmountDrive(void)
{
    f_mount(NULL, "", 0);
    Print_Msg("USB Drive unmounted\r\n");
}

void USB_ListRootDirectory(void)
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
            Print_Msg(msg);
        }
        else
        {
            USB_ReadWavHeader(fno.fname);
        }
    }

    f_closedir(&dir);
}

void USB_ReadWavHeader(const char *filename)
{
    FRESULT res;
    UINT br;
    char msg[100];

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK)
    {
        sprintf(msg, "Failed to open %s: %d\r\n", filename, res);
        Print_Msg(msg);
        return;
    }

    uint8_t header[44];
    res = f_read(&file, header, 44, &br);
    if (res != FR_OK || br != 44)
    {
        sprintf(msg, "Failed to read header of %s (read %u bytes)\r\n", filename, br);
        Print_Msg(msg);
        f_close(&file);
        return;
    }

    WAV_FormatTypeDef *wav = (WAV_FormatTypeDef *)header;

    sprintf(msg, "\r\n=== %s Header ===\r\n", filename);
    Print_Msg(msg);
    sprintf(msg, " File size     : %lu bytes\r\n", (unsigned long)wav->ChunkSize);
	Print_Msg(msg);
	sprintf(msg, " Format        : %c%c%c%c\r\n", (char)wav->Format, (char)(wav->Format>>8), (char)(wav->Format>>16), (char)(wav->Format>>24));
	Print_Msg(msg);
	sprintf(msg, " Sample Rate   : %lu Hz\r\n", (unsigned long)wav->SampleRate);
	Print_Msg(msg);
	sprintf(msg, " Channels      : %u\r\n", (unsigned)wav->NumChannels);
	Print_Msg(msg);
	sprintf(msg, " Bits/Sample   : %u\r\n", (unsigned)wav->BitPerSample);
	Print_Msg(msg);
	sprintf(msg, " ByteRate      : %lu\r\n", (unsigned long)wav->ByteRate);
	Print_Msg(msg);
	sprintf(msg, " Data ID       : %c%c%c%c\r\n", (char)wav->SubChunk2ID, (char)(wav->SubChunk2ID>>8), (char)(wav->SubChunk2ID>>16), (char)(wav->SubChunk2ID>>24));
	Print_Msg(msg);
	sprintf(msg, " Data size     : %lu bytes\r\n", (unsigned long)wav->SubChunk2Size);
	Print_Msg(msg);
	sprintf(msg, " Data size hex : 0x%04lx bytes\r\n", wav->SubChunk2Size);
	Print_Msg(msg);
	uint32_t duration = 0;
	if (wav->ByteRate > 0)
	{
		duration = wav->ChunkSize / wav->ByteRate;
		sprintf(msg, " Duration      : %02lu:%02lu\r\n", duration / 60, duration % 60);
		Print_Msg(msg);
	}

    f_close(&file);
}

void FS_Read(void)
{
    char msg[64];
    sprintf(msg, "FS_Read started\r\n");
    Print_Msg(msg);

    if (f_mount(&fs, "", 0) != FR_OK)
    {
        if (USB_MountDrive() != FR_OK)
            return;
    }

    USB_ListRootDirectory();
}
