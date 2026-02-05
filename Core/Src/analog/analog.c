#include "analog.h"
#include "stm32h7xx_hal.h"

uint16_t adc_buff[ADC_BUFF_SIZE];

bool cmp_with_delta(uint16_t v1, uint16_t v2, uint16_t delta)
{
	uint16_t dif = (v1 > v2) ? (v1 - v2) : (v2 - v1);
	return (dif < delta);
}

uint16_t get_adc_value(ADC_channel n_ch)
{
  return adc_buff[n_ch];
}

bool test_adc_value(ADC_channel n_ch, uint16_t value_mv)
{
	uint16_t val_adc = get_adc_value(n_ch);
	return cmp_with_delta(val_adc, value_mv, value_mv/20);
}

uint16_t getRData(uint16_t vDiv, uint16_t rawData)
{
	return vDiv * __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, rawData, LL_ADC_RESOLUTION_16B);
}

uint16_t getRDataD(uint16_t vDiv, uint16_t rawData)
{
	uint32_t val = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, rawData, LL_ADC_RESOLUTION_16B);
	return (uint16_t)((vDiv * val) / 1024);
}

