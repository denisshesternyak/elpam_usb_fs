#include <audio_cmd.h>
#include <audio_regs.h>
#include <cmsis_os.h>

extern I2C_HandleTypeDef hi2c5;
static void audio_cmd_write_cmd(uint8_t reg, uint8_t value);
static uint8_t audio_cmd_read_cmd(uint8_t reg);

static void audio_cmd_write_cmd(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
}

static uint8_t audio_cmd_read_cmd(uint8_t reg)
{
	uint8_t value = 0;
	HAL_I2C_Mem_Read(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
	return value;
}

void audio_cmd_reset(void)
{
	audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
	audio_cmd_write_cmd(AIC32X4_RESET, 0x01);			// Software reset
}

void audio_cmd_init_power(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
    audio_cmd_write_cmd(AIC32X4_PWRCFG, 0x08);    		// Disable weak AVDD connection
    audio_cmd_write_cmd(AIC32X4_LDOCTL, 0x00);   		// Master analog power control (LDO off if external supply)
    audio_cmd_write_cmd(AIC32X4_CMMODE, 0x00);			// Input common mode 0.9 V
    audio_cmd_write_cmd(AIC32X4_REFPOWERUP, 0x01); 		// REF charging time 40 ms
}

void audio_cmd_init_playback(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_CLKMUX, 0x00);			// CODEC_CLKIN = MCLK directly (no PLL)

	audio_cmd_write_cmd(AIC32X4_NDAC, 0x81);			// NDAC = 1, powered up
	audio_cmd_write_cmd(AIC32X4_MDAC, 0x82);			// MDAC = 2, powered up
	audio_cmd_write_cmd(AIC32X4_DOSRMSB, 0x00);			// DOSR = 1024
	audio_cmd_write_cmd(AIC32X4_DOSRLSB, 0x80);			// DOSR = 128

	audio_cmd_write_cmd(AIC32X4_IFACE1, 0x01); 			// 0x00 = I2S, 16-bit, BCLK/WCLK as inputs (slave)
	audio_cmd_write_cmd(AIC32X4_DACSPB, 0x08);   		// PRB_P8 for DAC (simple playback)

    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
    audio_cmd_write_cmd(AIC32X4_HEADSTART, 0x25);		// Headphone soft stepping (pop reduction)
    audio_cmd_write_cmd(AIC32X4_HPLROUTE, 0x08); 		// LDAC → HPL
	audio_cmd_write_cmd(AIC32X4_HPRROUTE, 0x08);   		// RDAC → HPR
	audio_cmd_write_cmd(AIC32X4_LPLAYBACK, 0x00);		// Playback Configuration Register 1
	audio_cmd_write_cmd(AIC32X4_RPLAYBACK, 0x00);		// Playback Configuration Register 2
	audio_cmd_write_cmd(AIC32X4_HPLGAIN, 0x00);   		// HPL gain 0 dB
	audio_cmd_write_cmd(AIC32X4_HPRGAIN, 0x00);			// HPR gain 0 dB
//	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x30); 		// Power up HPL and HPR drivers

	audio_cmd_write_cmd(AIC32X4_LOLROUTE, 0x08); 		// LDAC → LOL
	audio_cmd_write_cmd(AIC32X4_LORROUTE, 0x08);   		// RDAC → LOR
	audio_cmd_write_cmd(AIC32X4_LOLGAIN, 0x00);   		// LOL gain 0 dB, driver is not muted
	audio_cmd_write_cmd(AIC32X4_LORGAIN, 0x00);			// LOR gain 0 dB, driver is not muted
//	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x0C); 		// Power up LOL and LOR drivers

	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x00); 		// Power down HPL/HPR and LOL/LOR drivers

    osDelay(2500);								// Wait for soft stepping (2.5 sec in TI example)

    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    // 0xC8 -28dB,
    audio_cmd_write_cmd(AIC32X4_LDACVOL, 0x81); 		// Left DAC Channel Digital Volume -63.5dB
    audio_cmd_write_cmd(AIC32X4_RDACVOL, 0x81); 		// Right DAC Channel Digital Volume -63.5dB
    audio_cmd_write_cmd(AIC32X4_DACSETUP, 0xD6); 		// LDAC + RDAC powered, soft step 1/fs
    audio_cmd_write_cmd(AIC32X4_DACMUTE, 0x0C); 		// Mute both DACs, gain 0 dB
}

void audio_cmd_init_record(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_NADC, 0x81);   			// NADC = 1, powered up
    audio_cmd_write_cmd(AIC32X4_MADC, 0x82);     		// MADC = 2, powered up
    audio_cmd_write_cmd(AIC32X4_AOSR, 0x80);     		// AOSR = 128
    audio_cmd_write_cmd(AIC32X4_ADCSPB, 0x01);  		// PRB_R1 for ADC (default recording)

    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
    audio_cmd_write_cmd(AIC32X4_ADCPWTUNE, 0x00);		// Select ADC PTM_R4
    audio_cmd_write_cmd(AIC32X4_INPWRCTRL, 0x32);  		// MicPGA startup delay ~3.1 ms

    audio_cmd_write_cmd(AIC32X4_LMICPGAPIN, 0x80);  	// IN1L to LEFT_P, 20 kΩ
    audio_cmd_write_cmd(AIC32X4_LMICPGANIN, 0x80); 		// CM to LEFT_M, 20 kΩ
    audio_cmd_write_cmd(AIC32X4_RMICPGAPIN, 0x80);  	// IN1R to RIGHT_P, 20 kΩ
    audio_cmd_write_cmd(AIC32X4_RMICPGANIN, 0x80); 		// CM to RIGHT_M, 20 kΩ

    audio_cmd_write_cmd(AIC32X4_LMICPGAVOL, 0x0C);  	// Left MicPGA unmute, +6 dB
    audio_cmd_write_cmd(AIC32X4_RMICPGAVOL, 0x0C);		// Right MicPGA unmute, +6 dB

    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_ADCSETUP, 0x00); 		// Left + Right ADC powered up
    audio_cmd_write_cmd(AIC32X4_ADCMUTE, 0x88);     	// Mute both ADCs, gain 0 dB
}

void audio_cmd_send_volume(uint8_t lvl)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
	audio_cmd_write_cmd(AIC32X4_LDACVOL, lvl); 			// Left DAC Channel Digital Volume
	audio_cmd_write_cmd(AIC32X4_RDACVOL, lvl); 			// Right DAC Channel Digital Volume
}

void audio_cmd_enable_HP(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x30); 		// Power up HPL and HPR drivers
	osDelay(2);
}

void audio_cmd_enable_LO(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 1
	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x0C); 		// Power up LOL and LOR drivers
	osDelay(2);
}

void audio_cmd_playback_unmute(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_DACMUTE, 0x00); 		// Unute both DACs
}

void audio_cmd_playback_mute(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_DACMUTE, 0x0C); 		// Mute both DACs
}

void audio_cmd_playback_pwr_up(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 0
	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x3C); 		// Power up HPL/HPR and LOL/LOR drivers
}

void audio_cmd_playback_pwr_down(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x01);			// Page 0
	audio_cmd_write_cmd(AIC32X4_OUTPWRCTL, 0x00); 		// Power down HPL/HPR and LOL/LOR drivers
}

void audio_cmd_playback_enable(void)
{
	audio_cmd_playback_pwr_up();
	audio_cmd_playback_unmute();
}

void audio_cmd_playback_disable(void)
{
	audio_cmd_playback_pwr_down();
	audio_cmd_playback_mute();
}

////////////////////////////////////////////////////
void audio_cmd_microphone_unmute(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_ADCMUTE, 0x00);     	// Unmute both ADCs, gain 0 dB
}

void audio_cmd_microphone_mute(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_ADCMUTE, 0x88);     	// Mute both ADCs, gain 0 dB
}

void audio_cmd_microphone_pwr_up(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_ADCSETUP, 0xC0); 		// Left + Right ADC powered up
}

void audio_cmd_microphone_pwr_down(void)
{
    audio_cmd_write_cmd(AIC32X4_PSEL, 0x00);			// Page 0
    audio_cmd_write_cmd(AIC32X4_ADCSETUP, 0x00); 		// Left + Right ADC powered up
}

void audio_cmd_microphone_enable(void)
{
	audio_cmd_microphone_pwr_up();
	audio_cmd_microphone_unmute();
}

void audio_cmd_microphone_disable(void)
{
	audio_cmd_microphone_pwr_down();
	audio_cmd_microphone_mute();
}

