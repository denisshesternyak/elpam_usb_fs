#ifndef INC_CODEC_AUDIOFS_H_
#define INC_CODEC_AUDIOFS_H_

#include "fatfs.h"
#include "main.h"
#include "stdbool.h"

void audiofs_init(void);
bool audiofs_load_file(void);
void audiofs_close_file(void);
UINT audiofs_read_buffer_part(uint8_t *buffer, uint32_t buffer_len);

#endif /* INC_CODEC_AUDIOFS_H_ */
