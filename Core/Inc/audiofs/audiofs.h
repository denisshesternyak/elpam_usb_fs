#ifndef INC_CODEC_AUDIOFS_H_
#define INC_CODEC_AUDIOFS_H_

#include "fatfs.h"
#include "main.h"
#include "stdbool.h"
#include "audio_types.h"

FRESULT audiofs_mount_drive(void);
void audiofs_unmount_drive(void);
void audiofs_list_root_directory(void);
bool audiofs_read_file_info(AudioFileInfo_t* info);
bool audiofs_read_file(AudioFileInfo_t* info, uint8_t *buffer, uint32_t buffer_len);
void audiofs_close_file(AudioFileInfo_t* info);

#endif /* INC_CODEC_AUDIOFS_H_ */
