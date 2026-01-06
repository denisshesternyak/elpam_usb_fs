#include "tlv320aic3254.h"
#include "tlv320aic3254_cmd.h"
#include "tlv320aic3254_regs.h"
#include "math.h"
//#include "test_audio.h"
#include "string.h"
#include "audiofs.h"

extern I2S_HandleTypeDef hi2s2;

//#define SAMPLE_RATE         48000
//#define TONE_FREQ           826
//#define SAMPLES_PER_PERIOD  (SAMPLE_RATE / TONE_FREQ)
//#define PERIODS_IN_BUFFER   1
//#define BUFFER_SIZE         (SAMPLES_PER_PERIOD * PERIODS_IN_BUFFER)
//#define STEREO_WORDS 		(BUFFER_SIZE * 2)
//
//int16_t sine_buffer[STEREO_WORDS];
//uint32_t audio_repeat = 1;
//
//static void Generate_Sine_800Hz(void);
static void AIC3204_Reset(void);
static void AIC3204_Init_Playback(void);
static void AIC3204_Init_Record(void);

//static void Generate_Sine_800Hz(void)
//{
//	const char *msg = "Generate_Sine_800Hz\r\n";
//	Print_Msg(msg);
//
//    for (int sample_idx = 0; sample_idx < BUFFER_SIZE; sample_idx++)
//    {
//        double t = (double)sample_idx / SAMPLE_RATE;
//        double sample = sin(2.0 * M_PI * TONE_FREQ * t);
//        int16_t value = (int16_t)(sample * 32767.0);
//
//        int buffer_idx = sample_idx * 2;
//        sine_buffer[buffer_idx]   = value;   // Left
//        sine_buffer[buffer_idx+1] = value;   // Right
//    }
//}

static void AIC3204_Reset(void)
{
	const char *msg = "Reset\r\n";
	Print_Msg(msg);

	AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
	AIC3204_WriteReg(AIC32X4_RESET, 0x01);			// Software reset
}

static void AIC3204_Init_Playback(void)
{
	const char *msg = "Init_Playback\r\n";
	Print_Msg(msg);

	HAL_Delay(10);
	AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
	AIC3204_WriteReg(AIC32X4_RESET, 0x01);			// Software reset
	HAL_Delay(50);

    AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
    AIC3204_WriteReg(AIC32X4_CLKMUX, 0x00);			// CODEC_CLKIN = MCLK directly (no PLL)

	AIC3204_WriteReg(AIC32X4_NDAC, 0x81);			// NDAC = 1, powered up
	AIC3204_WriteReg(AIC32X4_MDAC, 0x82);			// MDAC = 2, powered up
	AIC3204_WriteReg(AIC32X4_DOSRMSB, 0x00);		// DOSR = 1024
	AIC3204_WriteReg(AIC32X4_DOSRLSB, 0x80);		// DOSR = 128

	AIC3204_WriteReg(AIC32X4_IFACE1, 0x01); 		// 0x00 = I2S, 16-bit, BCLK/WCLK as inputs (slave)
	AIC3204_WriteReg(AIC32X4_DACSPB, 0x08);   		// PRB_P8 for DAC (simple playback)

    AIC3204_WriteReg(AIC32X4_PSEL, 0x01);			// Page 1
    AIC3204_WriteReg(AIC32X4_PWRCFG, 0x08);  		// Disable weak AVDD connection
    AIC3204_WriteReg(AIC32X4_LDOCTL, 0x00);   		// Master analog power control (LDO off if external supply)
    AIC3204_WriteReg(AIC32X4_REFPOWERUP, 0x01);	 	// Reference power up with 40ms charge time
    AIC3204_WriteReg(AIC32X4_HEADSTART, 0x25);		// Headphone soft stepping (pop reduction)
    AIC3204_WriteReg(AIC32X4_CMMODE, 0x00);   		// Input common mode 0.9 V
    AIC3204_WriteReg(AIC32X4_HPLROUTE, 0x08); 		// LDAC → HPL
	AIC3204_WriteReg(AIC32X4_HPRROUTE, 0x08);   	// RDAC → HPR
	AIC3204_WriteReg(AIC32X4_LPLAYBACK, 0x00);		// Playback Configuration Register 1
	AIC3204_WriteReg(AIC32X4_RPLAYBACK, 0x00);		// Playback Configuration Register 2
	AIC3204_WriteReg(AIC32X4_HPLGAIN, 0x00);   		// HPL gain 0 dB
	AIC3204_WriteReg(AIC32X4_HPRGAIN, 0x00);		// HPR gain 0 dB
	AIC3204_WriteReg(AIC32X4_OUTPWRCTL, 0x30); 		// Power up HPL and HPR drivers

    HAL_Delay(2500);								// Wait for soft stepping (2.5 sec in TI example)

    AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
    AIC3204_WriteReg(AIC32X4_LDACVOL, 0x00); 		// Left DAC Channel Digital Volume 0.0dB
    AIC3204_WriteReg(AIC32X4_RDACVOL, 0x00); 		// Right DAC Channel Digital Volume 0.0dB
    AIC3204_WriteReg(AIC32X4_DACSETUP, 0xD6); 		// LDAC + RDAC powered, soft step 1/fs
    AIC3204_WriteReg(AIC32X4_DACMUTE, 0x00); 		// Unmute both DACs, gain 0 dB
}

static void AIC3204_Init_Record(void)
{
	const char *msg = "Init_Record\r\n";
	Print_Msg(msg);

    AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
    AIC3204_WriteReg(AIC32X4_NADC, 0x81);   		// NADC = 1, powered up
    AIC3204_WriteReg(AIC32X4_MADC, 0x82);     		// MADC = 2, powered up
    AIC3204_WriteReg(AIC32X4_AOSR, 0x80);     		// AOSR = 128
    AIC3204_WriteReg(AIC32X4_ADCSPB, 0x01);  		// PRB_R1 for ADC (default recording)

    AIC3204_WriteReg(AIC32X4_PSEL, 0x01);			// Page 0
    AIC3204_WriteReg(AIC32X4_PWRCFG, 0x08);    		// Disable weak AVDD connection
    AIC3204_WriteReg(AIC32X4_LDOCTL, 0x00);   		// Master analog power control (LDO off if external supply)
    AIC3204_WriteReg(AIC32X4_CMMODE, 0x00);			// Input common mode 0.9 V
    AIC3204_WriteReg(AIC32X4_ADCPWTUNE, 0x00);		// Select ADC PTM_R4
    AIC3204_WriteReg(AIC32X4_INPWRCTRL, 0x32);  	// MicPGA startup delay ~3.1 ms
    AIC3204_WriteReg(AIC32X4_REFPOWERUP, 0x01); 	// REF charging time 40 ms

    AIC3204_WriteReg(AIC32X4_LMICPGAPIN, 0x80);  	// IN1L to LEFT_P, 20 kΩ
    AIC3204_WriteReg(AIC32X4_LMICPGANIN, 0x80); 	// CM to LEFT_M, 20 kΩ
    AIC3204_WriteReg(AIC32X4_RMICPGAPIN, 0x80);  	// IN1R to RIGHT_P, 20 kΩ
    AIC3204_WriteReg(AIC32X4_RMICPGANIN, 0x80); 	// CM to RIGHT_M, 20 kΩ

    AIC3204_WriteReg(AIC32X4_LMICPGAVOL, 0x0C);  	// Left MicPGA unmute, +6 dB
    AIC3204_WriteReg(AIC32X4_RMICPGAVOL, 0x0C);		// Right MicPGA unmute, +6 dB

    AIC3204_WriteReg(AIC32X4_PSEL, 0x00);			// Page 0
    AIC3204_WriteReg(AIC32X4_ADCSETUP, 0xC0); 		// Left + Right ADC powered up
    AIC3204_WriteReg(AIC32X4_ADCMUTE, 0x00);     	// Unmute both ADCs, gain 0 dB
}

void AIC3204_Init_Audio(void)
{
	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	HAL_I2S_Init(&hi2s2);

	//Generate_Sine_800Hz();

	/*HAL_Delay(10);
	AIC3204_Reset();

	HAL_Delay(50);*/

	AIC3204_Init_Playback();
	//AIC3204_Init_Record();

	//FS_ReadDisk();

//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);
//	HAL_Delay(1000);
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
//	HAL_Delay(1000);
//
//	Print_Msg("AIC3204_Init_Audio\r\n");

	FS_MountDrive();
}

void AIC3204_Start_Playback(const char *filename)
{
	Print_Msg("AIC3204_Play\r\n");

    if (!FS_LoadFile(filename))
	{
    	char msg[64];
    	sprintf(msg, "Failure load %s\r\n", filename);
    	Print_Msg(msg);
    	return;
	}
    player.is_playing = true;

	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	HAL_I2S_Init(&hi2s2);
    HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)player.dma_buffer, AUDIO_HALF_BUFFER_SIZE);
}

void AIC3204_Stop_Playback(void)
{
	char msg[64];
	sprintf(msg, "All read %ld bytes\r\n", player.bytes_read);
	Print_Msg(msg);

	player.is_playing = false;
	player.buff_state = BUFFER_IDLE;
	player.bytes_read = 0;

    HAL_I2S_DMAStop(&hi2s2);
    hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    HAL_I2S_Init(&hi2s2);

    FS_CloseFile();
}

void AIC3204_Process(void)
{
	if (!player.is_playing || !player.file_opened || player.buff_state == BUFFER_IDLE) return;

	UINT br = FS_Read_Buffer_Part();
	if (br < 0) return;

	if (br == 0)
	{
		AIC3204_Stop_Playback();
	}
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance == SPI2)
	{
		if (player.is_playing)
		{
			player.buff_state = BUFFER_HALF;
		}
	}
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance == SPI2)
	{
		if (player.is_playing)
		{
			player.buff_state = BUFFER_FULL;
		}
	}
}
/*
void AIC3204_Start_Record(void)
{
	const char *msg = "Start_Record\r\n";
	Print_Msg(msg);
}

void AIC3204_Stop_Record(void)
{
	const char *msg = "Stop_Record\r\n";
	Print_Msg(msg);
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {

    }
}
*/

