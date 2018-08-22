/*
 * es8374.h
 *
 *  Created on: 2018年4月28日
 *      Author: ydg
 */

#ifndef PROJECT_MT7686_HDK_HAL_EXAMPLES_I2C_CONTROL_EEPROM_WITH_POLLING_INC_ES8374_H_
#define PROJECT_MT7686_HDK_HAL_EXAMPLES_I2C_CONTROL_EEPROM_WITH_POLLING_INC_ES8374_H_
#include "hal_i2c_master.h"


#define ES8374_I2C_ADDR            (0x20>>1)
#define es8374_id                   0xe0
//hal_i2c_port_t es8374_i2c_port;

/**  This enum define the HAL interface return value. */
typedef enum {
    CODEC_STATUS_ERROR = -1,   /**<  An error occurred and the transaction failed. */
    CODEC_STATUS_OK = 0        /**<  No error occurred during the function call. */
} CODEC_STATUS_e;


CODEC_STATUS_e es8374_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency);
CODEC_STATUS_e es8374_i2c_deinit(void);
uint8_t i2c_write(uint8_t u8Addr,uint8_t u8Data);
uint8_t es8374_init(void);
#endif /* PROJECT_MT7686_HDK_HAL_EXAMPLES_I2C_CONTROL_EEPROM_WITH_POLLING_INC_ES8374_H_ */
