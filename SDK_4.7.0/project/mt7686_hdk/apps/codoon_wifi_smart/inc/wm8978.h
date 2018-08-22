/*
 * wm8978.h
 *
 *  Created on: 2018年5月28日
 *      Author: ydg
 */

#ifndef PROJECT_MT7686_HDK_APPS_AUDIO_REF_DESIGN_INC_WM8978_H_
#define PROJECT_MT7686_HDK_APPS_AUDIO_REF_DESIGN_INC_WM8978_H_

#include "hal_i2c_master.h"


#define WM8978_I2C_ADDR            (0x34>>1)
#define wm8978_id                   0xe0
//static hal_i2c_port_t wm8978_i2c_port;

/**  This enum define the HAL interface return value. */
typedef enum {
	WM8978_CODEC_STATUS_ERROR = -1,   /**<  An error occurred and the transaction failed. */
	WM8978_CODEC_STATUS_OK = 0        /**<  No error occurred during the function call. */
} WM8978_CODEC_STATUS_e;


WM8978_CODEC_STATUS_e wm8978_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency);
WM8978_CODEC_STATUS_e wm8978_i2c_deinit(void);
uint8_t wm8978_i2c_write(uint8_t u8Addr,uint16_t u8Data);
uint16_t wm8978_i2c_read(uint8_t reg);
uint8_t wm8978_init(void);
void WM8978_I2S_Cfg(uint8_t fmt,uint8_t len);
void WM8978_SPKvol_Set(uint8_t volx);
void WM8978_HPvol_Set(uint8_t voll,uint8_t volr);
void WM8978_Output_Cfg(uint8_t dacen,uint8_t bpsen);
void WM8978_AUX_Gain(uint8_t gain);
void WM8978_LINEIN_Gain(uint8_t gain);
void WM8978_MIC_Gain(uint8_t gain);
void WM8978_Input_Cfg(uint8_t micen,uint8_t lineinen,uint8_t auxen);
void WM8978_ADDA_Cfg(uint8_t dacen,uint8_t adcen);

#endif /* PROJECT_MT7686_HDK_APPS_AUDIO_REF_DESIGN_INC_WM8978_H_ */
