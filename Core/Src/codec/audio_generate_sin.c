#include "audio_generate_sin.h"
#include "math.h"

#define WAVE_TABLE_SIZE 1024
#define SAMPLE_RATE     48000

#define FREQ_CH1        826
#define FREQ_CH2        865

#define PHASE_INC_CH1	73909228U	//(1U << 32) / (SAMPLE_RATE / FREQ_CH1)
#define PHASE_INC_CH2	77398889U	//(1U << 32) / (SAMPLE_RATE / FREQ_CH1)

static int16_t wave_table[WAVE_TABLE_SIZE];
static uint32_t phase_acc_ch1 = 0;
static uint32_t phase_acc_ch2 = 0;

void audio_init_wave_table(void)
{    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_table[i] = (int16_t)(sinf(M_TWOPI * i / WAVE_TABLE_SIZE) * 32767);
    }
}

void audio_generate_sine(uint8_t *buffer, uint32_t samples_per_channel)
{
    for (uint32_t i = 0; i < samples_per_channel; i++)
    {
        uint32_t index_ch1 = (phase_acc_ch1 >> 22) & (WAVE_TABLE_SIZE - 1);
        uint32_t index_ch2 = (phase_acc_ch2 >> 22) & (WAVE_TABLE_SIZE - 1);

        int16_t sample_left = wave_table[index_ch1];
        int16_t sample_right = wave_table[index_ch2];

        uint32_t buffer_idx = i * 4;

        buffer[buffer_idx]     = sample_left & 0xFF;        	// Left LSB
        buffer[buffer_idx + 1] = (sample_left >> 8) & 0xFF; 	// Left MSB
        buffer[buffer_idx + 2] = sample_right & 0xFF;       	// Right LSB
        buffer[buffer_idx + 3] = (sample_right >> 8) & 0xFF;	// Right MSB

        phase_acc_ch1 += PHASE_INC_CH1;
        phase_acc_ch2 += PHASE_INC_CH2;
    }
}
