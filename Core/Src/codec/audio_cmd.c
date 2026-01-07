#include <audio_cmd.h>
#include <audio_regs.h>

extern I2C_HandleTypeDef hi2c5;
static void audio_write_cmd(uint8_t reg, uint8_t value);
static uint8_t audio_read_cmd(uint8_t reg);

static void audio_write_cmd(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
}

static uint8_t audio_read_cmd(uint8_t reg)
{
	uint8_t value;
	HAL_I2C_Mem_Read(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
	return value;
}

void audio_reset(void)
{
	const char *msg = "audio_reset\r\n";
	Print_Msg(msg);

	audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
	audio_write_cmd(AIC32X4_RESET, 0x01);			// Software reset
}

void audio_init_playback(void)
{
	const char *msg = "audio_init_playback\r\n";
	Print_Msg(msg);

	HAL_Delay(10);
	audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
	audio_write_cmd(AIC32X4_RESET, 0x01);			// Software reset
	HAL_Delay(50);

    audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_write_cmd(AIC32X4_CLKMUX, 0x00);			// CODEC_CLKIN = MCLK directly (no PLL)

	audio_write_cmd(AIC32X4_NDAC, 0x81);			// NDAC = 1, powered up
	audio_write_cmd(AIC32X4_MDAC, 0x82);			// MDAC = 2, powered up
	audio_write_cmd(AIC32X4_DOSRMSB, 0x00);			// DOSR = 1024
	audio_write_cmd(AIC32X4_DOSRLSB, 0x80);			// DOSR = 128

	audio_write_cmd(AIC32X4_IFACE1, 0x01); 			// 0x00 = I2S, 16-bit, BCLK/WCLK as inputs (slave)
	audio_write_cmd(AIC32X4_DACSPB, 0x08);   		// PRB_P8 for DAC (simple playback)

    audio_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
    audio_write_cmd(AIC32X4_PWRCFG, 0x08);  		// Disable weak AVDD connection
    audio_write_cmd(AIC32X4_LDOCTL, 0x00);   		// Master analog power control (LDO off if external supply)
    audio_write_cmd(AIC32X4_REFPOWERUP, 0x01);	 	// Reference power up with 40ms charge time
    audio_write_cmd(AIC32X4_HEADSTART, 0x25);		// Headphone soft stepping (pop reduction)
    audio_write_cmd(AIC32X4_CMMODE, 0x00);   		// Input common mode 0.9 V
    audio_write_cmd(AIC32X4_HPLROUTE, 0x08); 		// LDAC → HPL
	audio_write_cmd(AIC32X4_HPRROUTE, 0x08);   		// RDAC → HPR
	audio_write_cmd(AIC32X4_LPLAYBACK, 0x00);		// Playback Configuration Register 1
	audio_write_cmd(AIC32X4_RPLAYBACK, 0x00);		// Playback Configuration Register 2
	audio_write_cmd(AIC32X4_HPLGAIN, 0x00);   		// HPL gain 0 dB
	audio_write_cmd(AIC32X4_HPRGAIN, 0x00);			// HPR gain 0 dB
	audio_write_cmd(AIC32X4_OUTPWRCTL, 0x30); 		// Power up HPL and HPR drivers

    HAL_Delay(2500);								// Wait for soft stepping (2.5 sec in TI example)

    audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    // 0xC8 -28dB,
    audio_write_cmd(AIC32X4_LDACVOL, 0x00); 		// Left DAC Channel Digital Volume 0.0dB
    audio_write_cmd(AIC32X4_RDACVOL, 0x00); 		// Right DAC Channel Digital Volume 0.0dB
    audio_write_cmd(AIC32X4_DACSETUP, 0xD6); 		// LDAC + RDAC powered, soft step 1/fs
    audio_write_cmd(AIC32X4_DACMUTE, 0x00); 		// Unmute both DACs, gain 0 dB
}

void audio_init_record(void)
{
	const char *msg = "Init_Record\r\n";
	Print_Msg(msg);

    audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_write_cmd(AIC32X4_NADC, 0x81);   			// NADC = 1, powered up
    audio_write_cmd(AIC32X4_MADC, 0x82);     		// MADC = 2, powered up
    audio_write_cmd(AIC32X4_AOSR, 0x80);     		// AOSR = 128
    audio_write_cmd(AIC32X4_ADCSPB, 0x01);  		// PRB_R1 for ADC (default recording)

    audio_write_cmd(AIC32X4_PSEL, 0x01);			// Page 0
    audio_write_cmd(AIC32X4_PWRCFG, 0x08);    		// Disable weak AVDD connection
    audio_write_cmd(AIC32X4_LDOCTL, 0x00);   		// Master analog power control (LDO off if external supply)
    audio_write_cmd(AIC32X4_CMMODE, 0x00);			// Input common mode 0.9 V
    audio_write_cmd(AIC32X4_ADCPWTUNE, 0x00);		// Select ADC PTM_R4
    audio_write_cmd(AIC32X4_INPWRCTRL, 0x32);  		// MicPGA startup delay ~3.1 ms
    audio_write_cmd(AIC32X4_REFPOWERUP, 0x01); 		// REF charging time 40 ms

    audio_write_cmd(AIC32X4_LMICPGAPIN, 0x80);  	// IN1L to LEFT_P, 20 kΩ
    audio_write_cmd(AIC32X4_LMICPGANIN, 0x80); 		// CM to LEFT_M, 20 kΩ
    audio_write_cmd(AIC32X4_RMICPGAPIN, 0x80);  	// IN1R to RIGHT_P, 20 kΩ
    audio_write_cmd(AIC32X4_RMICPGANIN, 0x80); 		// CM to RIGHT_M, 20 kΩ

    audio_write_cmd(AIC32X4_LMICPGAVOL, 0x0C);  	// Left MicPGA unmute, +6 dB
    audio_write_cmd(AIC32X4_RMICPGAVOL, 0x0C);		// Right MicPGA unmute, +6 dB

    audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_write_cmd(AIC32X4_ADCSETUP, 0xC0); 		// Left + Right ADC powered up
    audio_write_cmd(AIC32X4_ADCMUTE, 0x00);     	// Unmute both ADCs, gain 0 dB
}

void audio_send_volume(uint8_t lvl)
{
    audio_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
	audio_write_cmd(AIC32X4_LDACVOL, lvl); 		// Left DAC Channel Digital Volume
	audio_write_cmd(AIC32X4_RDACVOL, lvl); 		// Right DAC Channel Digital Volume
}
