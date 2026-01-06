#ifndef INC_CODEC_TLV320AIC3254_H_
#define INC_CODEC_TLV320AIC3254_H_

#include "main.h"
#include "stdbool.h"

void AIC3204_Init_Audio(void);
void AIC3204_Start_Playback(const char *filename);
void AIC3204_Stop_Playback(void);

void AIC3204_Start_Record(void);
void AIC3204_Stop_Record(void);
void AIC3204_Process(void);

#endif /* INC_CODEC_TLV320AIC3254_H_ */
