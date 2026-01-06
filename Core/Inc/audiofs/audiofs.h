#ifndef INC_CODEC_AUDIOFS_H_
#define INC_CODEC_AUDIOFS_H_

#include "fatfs.h"
#include "main.h"
#include "stdbool.h"

//void FS_ReadDisk(void);
FRESULT audiofs_mount_drive(void);
void audiofs_unmount_drive(void);
bool audiofs_load_file(const char* filename);
void audiofs_close_file(void);
UINT audiofs_read_buffer_part();

#endif /* INC_CODEC_AUDIOFS_H_ */
