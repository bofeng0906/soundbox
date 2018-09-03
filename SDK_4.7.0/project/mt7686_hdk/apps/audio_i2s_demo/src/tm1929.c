/******************************************************************************
//*版权信息：深圳市天微电子有限公司
//*文 件 名：TM1929-V1.0
//*当前版本：V1.0
//*MCU 型号：STC12C5608AD
//*开发环境：Keil uVision4
//*晶震频率：33MHZ(内部时钟)       
//*完成日期：2018-04-10
//*程序功能：1.该程序可实现TM1929白灯依次点亮。
//*免责声明：1.此程序为TM1929驱动LED点亮演示程序，仅作参考之用。
//           2.如有直接使用本例程程序造成经济损失的，本公司不承担任何责任。             
/*******************************************************************************/
#include "hal_gpt.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_log.h"
#include "hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mt768x_i2c.h"

#include "tm1929.h"

static int led1929_init_flag = 0;

int8_t led1929_i2c_write(
    uint8_t u8Addr,
    uint8_t u8Data)
{
	uint8_t u8TxDataBuf[2];

	hal_i2c_status_t i2c_status;
	i2c_status = HAL_I2C_STATUS_OK;

	u8TxDataBuf[0] = u8Addr;
	u8TxDataBuf[1] = u8Data;

	i2c_status = hal_i2c_master_send_polling(HAL_I2C_MASTER_1, LED1929_I2C_ADDR, u8TxDataBuf, 2);
	if (i2c_status != HAL_I2C_STATUS_OK) {
	    log_hal_error("[led1929]i2c write fail: RegAddress=%02x  Data=%u\n\r", u8TxDataBuf[0], u8TxDataBuf[1]);
	    return -1;
	}

	return 0;
}

uint8_t led1929_i2c_read(uint8_t u8Addr)
{
    uint8_t u8TxDataBuf;

    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;


    i2c_status = hal_i2c_master_send_polling(HAL_I2C_MASTER_1, LED1929_I2C_ADDR, &u8Addr, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[led1929][1]i2c read fail1: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    i2c_status = hal_i2c_master_receive_polling(HAL_I2C_MASTER_1, LED1929_I2C_ADDR, &u8TxDataBuf, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[led1929][2]i2c read fail2: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    return u8TxDataBuf;
}

static AUCODEC_STATUS_e aucodec_i2c_master_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency)
{
    /*I2C*/
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;

    //xSemaphoreTake(semaphore_i2c_port, portMAX_DELAY);

    i2c_status = HAL_I2C_STATUS_OK;
    i2c_config.frequency = (hal_i2c_frequency_t)frequency;

    i2c_status = hal_i2c_master_init(i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[led1929]i2c init ok\n");
    } else {
        log_hal_error("[led1929]i2c init error\n");
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}

void led1929_init(void)
{
	
	if(led1929_init_flag > 0)
		return 0;

	led1929_init_flag = 1;


    printf("led1929_init\n");
	
	//aucodec_i2c_master_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
	
	led1929_i2c_write(0x00,0x00);
	led1929_i2c_write(0x01,0x3F); //向地址01写入0x3f
	
}

/*************************************主函数**********************************/
void test_led1929(void)
{ 

	unsigned char i;
    int cnt = 0;
		
    while(1)
    { 
        cnt++;
        if(cnt>10)
        {
            break;
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x02,0x7F); //向亮度寄存器中写入亮度值
            led1929_i2c_write(0x03,0x7F); //向亮度寄存器中写入亮度值
            led1929_i2c_write(0x04,0x7F); //向亮度寄存器中写入亮度值
            vTaskDelay(10);
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x05,0x7F);
            led1929_i2c_write(0x06,0x7F);
            led1929_i2c_write(0x07,0x7F);
            vTaskDelay(10);
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x08,0x7F);
            led1929_i2c_write(0x09,0x7F);
            led1929_i2c_write(0x0A,0x7F);
            vTaskDelay(10);
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x0B,0x7F);		 
            led1929_i2c_write(0x0C,0x7F);
            led1929_i2c_write(0x0D,0x7F);
            vTaskDelay(10);
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x0E,0x7F);
            led1929_i2c_write(0x0F,0x7F);
            led1929_i2c_write(0x10,0x7F);
            vTaskDelay(10);
        }
        for(i=0;i<PWM_LEN;i++)          
        {
            led1929_i2c_write(0x11,0x7F);
            led1929_i2c_write(0x12,0x7F);
            led1929_i2c_write(0x13,0x7F);
            vTaskDelay(10);		
        }

        vTaskDelay(1000);	

    }		
 }
