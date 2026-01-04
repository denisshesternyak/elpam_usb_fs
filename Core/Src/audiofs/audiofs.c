#include "audiofs.h"
#include "fatfs.h"

typedef struct
{
  uint32_t   ChunkID;       /* 0 */
  uint32_t   FileSize;      /* 4 */
  uint32_t   FileFormat;    /* 8 */
  uint32_t   SubChunk1ID;   /* 12 */
  uint32_t   SubChunk1Size; /* 16 */
  uint16_t   AudioFormat;   /* 20 */
  uint16_t   NbrChannels;   /* 22 */
  uint32_t   SampleRate;    /* 24 */
  uint32_t   ByteRate;      /* 28 */
  uint16_t   BlockAlign;    /* 32 */
  uint16_t   BitPerSample;  /* 34 */
  uint32_t   SubChunk2ID;   /* 36 */
  uint32_t   SubChunk2Size; /* 40 */
}WAV_FormatTypeDef;

void FS_Read(void)
{
	FATFS fs;
	DIR dir;
	FILINFO fno;
	FIL file;
	FRESULT res;
	UINT br;
	WAV_FormatTypeDef *wavformat;
	uint32_t duration;

	char msg[64];

    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        return;
    }

    res = f_opendir(&dir, "/");
    if (res == FR_OK) {
    	sprintf(msg, "Root directory contents:\n");
    	Print_Msg(msg);
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;

            if (fno.fattrib & AM_DIR) {
            	sprintf(msg, "  <DIR>  %s\n", fno.fname);
            	Print_Msg(msg);
            } else {
            	sprintf(msg, "  %8lu %s\n", (unsigned long)fno.fsize, fno.fname);
            	Print_Msg(msg);
            }
        }
        f_closedir(&dir);
    } else {
        sprintf(msg, "Failed to open root directory: %d\n", res);
        Print_Msg(msg);
    }

    res = f_open(&file, "1.wav", FA_READ);
    if (res == FR_OK) {
        char buffer[44];
        sprintf(msg, "\nContents of 1.wav:\n");
        Print_Msg(msg);

        while (!f_eof(&file)) {
            res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
            if (res != FR_OK || br == 0) break;

			wavformat = (WAV_FormatTypeDef*) buffer;
			sprintf(msg,  " SampleRate %d\n", (int)(wavformat->SampleRate));
			Print_Msg(msg);

			sprintf(msg,  " NbrChannels %d\n", (int)(wavformat->NbrChannels));
			Print_Msg(msg);

			duration = wavformat->FileSize / wavformat->ByteRate;

			sprintf(msg, " FileSize %d\n", (int)(wavformat->FileSize/1024));
			Print_Msg(msg);

			sprintf(msg, " Duration: %02d:%02d\n", (int)(duration/60), (int)(duration%60));
			Print_Msg(msg);
        }

        f_close(&file);
    } else {
        sprintf(msg, "Failed to open file: %d\n", res);
        Print_Msg(msg);
    }
}
