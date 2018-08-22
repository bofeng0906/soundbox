/*
 * es8374.c
 *
 *  Created on: 2018年4月28日
 *      Author: ydg
 */
#include "hal_gpt.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_pinmux_define.h"
#include "memory_attribute.h"
#include "hal.h"
#include "hal_platform.h"
#include "es8374.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
/////////////////////////////////////////////////////////////////////////////////
hal_i2c_port_t es8374_i2c_port;
/////////////////////////////////////////////////////////////////////////////////
uint8_t i2c_write(uint8_t u8Addr,uint8_t u8Data)
{

    uint8_t u8TxDataBuf[2];
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;
    //hal_i2c_port_t i2c_port = es8374_i2c_port;

    u8TxDataBuf[0] = u8Addr;
    u8TxDataBuf[1] = u8Data;
    hal_gpt_delay_ms(2);
    i2c_status = hal_i2c_master_send_polling(es8374_i2c_port, ES8374_I2C_ADDR, u8TxDataBuf, 2);
    if (i2c_status != HAL_I2C_STATUS_OK)
    {
        printf("em8374 i2c write fail: RegAddress=%02x  Data=%02x \r\n", u8TxDataBuf[0], u8TxDataBuf[1]);
	    return 1;
    }
    if (i2c_status == HAL_I2C_STATUS_OK)
    {
    	printf("em8374 i2c write ok: RegAddress=%02x  Data=%02x \r\n", u8TxDataBuf[0], u8TxDataBuf[1]);
    	return 0;
    }
    return 0;
}
uint8_t  es8374_init(void)
{
	/*
    uint8_t res=0;

    res=i2c_write(0x00,0x3F); //IC Rst start
    if(res) return  1;
    i2c_write(0x00,0x03); //IC Rst stop
    i2c_write(0x01,0xFF); //IC clk on

    i2c_write(0x6F,0xA0); //pll set:mode enablHAL_I2C_MASTER_1e
    i2c_write(0x72,0x41); //pll set:mode set
    i2c_write(0x09,0x01); //pll set:reset on ,set start
    ///* PLL FOR 26MHZ/44.1KHZ
    i2c_write(0x0C,0x27); //pll set:k
    i2c_write(0x0D,0xDC); //pll set:k
    i2c_write(0x0E,0x2B); //pll set:k
    i2c_write(0x0A,0x8A); //pll set:
    i2c_write(0x0B,0x06); //pll set:n
    i2c_write(0x09,0x41); //pll set:reset off ,set stop
    i2c_write(0x05,0x11); //clk div =1
    i2c_write(0x03,0x20); //osr =32
    i2c_write(0x06,0x01); //LRCK div =0100H = 256D
    i2c_write(0x07,0x00);
    i2c_write(0x0F,0x04); //MASTER MODE, BCLK = MCLK/4
    i2c_write(0x10,0x0C); //I2S-16BIT, ADC
    i2c_write(0x11,0x0C); //I2S-16BIT, DAC
    //i2c_write(0x02,0x08);  //select PLL

    i2c_write(0x24,0x08); //adc set
    i2c_write(0x36,0x40); //dac set
    i2c_write(0x12,0x30); //timming set
    i2c_write(0x13,0x20); //timming set
    i2c_write(0x21,0x50); //adc set: SEL LIN1 CH+PGAGAIN=0DB
    i2c_write(0x22,0xFF); //adc set: PGA GAIN=0DB
    i2c_write(0x21,0x24); //adc set: SEL LIN1 CH+PGAGAIN=18DB
    i2c_write(0x22,0x55); //pga = +15db
    i2c_write(0x00,0x80); // IC START
    hal_gpt_delay_ms(50);
    i2c_write(0x14,0x8A); // IC START
    i2c_write(0x15,0x40); // IC START
    i2c_write(0x1A,0xA0); // monoout set
    i2c_write(0x1B,0x19); // monoout set
    i2c_write(0x1C,0x90); // spk set
    i2c_write(0x1D,0x2B); // spk set
    i2c_write(0x1F,0x00); // spk set
    i2c_write(0x1E,0xA2); // spk on
    i2c_write(0x28,0xa0); // alc set
    i2c_write(0x26,0x50);
    i2c_write(0x25,0x00); // ADCVOLUME on
    i2c_write(0x38,0x02); // DACVOLUMEL on
    i2c_write(0x37,0x30); // dac set
    i2c_write(0x6D,0x60); //SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT

    i2c_write(0x71,0x05);
    i2c_write(0x73,0x70);
    i2c_write(0x36,0x00); //dac set
    i2c_write(0x37,0x00); // dac set
    return 0;
    */
	uint8_t res=0;

	res=i2c_write(0x00,0x3F); //IC Rst start
	//if(res) return  1;
	i2c_write(0x00,0x03); //IC Rst stop
	i2c_write(0x01,0xFF); //IC clk on

	i2c_write(0x6F,0xA0); //pll set:mode enablHAL_I2C_MASTER_1e
	i2c_write(0x72,0x41); //pll set:mode set
	i2c_write(0x09,0x01); //pll set:reset on ,set start
	/* PLL FOR 26MHZ/44.1KHZ */
	i2c_write(0x0C,0x27); //pll set:k
	i2c_write(0x0D,0xDC); //pll set:k
	i2c_write(0x0E,0x2B); //pll set:k
	i2c_write(0x0A,0x8A); //pll set:
	i2c_write(0x0B,0x06); //pll set:n
	i2c_write(0x09,0x41); //pll set:reset off ,set stop
	i2c_write(0x05,0x11); //clk div =1
	i2c_write(0x03,0x20); //osr =32
	i2c_write(0x06,0x01); //LRCK div =0100H = 256D
	i2c_write(0x07,0x00);
	i2c_write(0x0F,0x04); //MASTER MODE, BCLK = MCLK/4
	i2c_write(0x10,0x0C); //I2S-16BIT, ADC
	i2c_write(0x11,0x0C); //I2S-16BIT, DAC
	i2c_write(0x02,0x08);  //select PLL

	i2c_write(0x24,0x08); //adc set
	i2c_write(0x36,0x40); //dac set
	i2c_write(0x12,0x30); //timming set
	i2c_write(0x13,0x20); //timming set
	i2c_write(0x21,0x50); //adc set: SEL LIN1 CH+PGAGAIN=0DB
	i2c_write(0x22,0xFF); //adc set: PGA GAIN=0DB
	i2c_write(0x21,0x24); //adc set: SEL LIN1 CH+PGAGAIN=18DB
	i2c_write(0x22,0x55); //pga = +15db
	i2c_write(0x00,0x80); // IC START
	hal_gpt_delay_ms(50);
	i2c_write(0x14,0x8A); // IC START
	i2c_write(0x15,0x40); // IC START
	i2c_write(0x1A,0xA0); // monoout set
	i2c_write(0x1B,0x19); // monoout set
	i2c_write(0x1C,0x90); // spk set
	i2c_write(0x1D,0x02); // spk set
	i2c_write(0x1F,0x00); // spk set
	i2c_write(0x1E,0xA0); // spk on
	i2c_write(0x28,0xa0); // alc set
	i2c_write(0x26,0x50);
	i2c_write(0x25,0x00); // ADCVOLUME on
	i2c_write(0x38,0x02); // DACVOLUMEL on
	i2c_write(0x37,0x30); // dac set
	i2c_write(0x6D,0x60); //SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT

	i2c_write(0x71,0x05);
	i2c_write(0x73,0x70);
	i2c_write(0x36,0x00); //dac set
	//i2c_write(0x36,0x80); //dac set
	i2c_write(0x37,0x00); // dac set
	return 0;
}

/*****************************************************************************************************************
 * @Function:
 *      es8374_i2c_init
 *
 * @Routine Description:
 *      i2c initial
 *
 * @Parameter:
 *                  hal_i2c_port_t i2c_port
 *                              HAL_I2C_MASTER_0 = 0
 *                              HAL_I2C_MASTER_1 = 1
 *                              HAL_I2C_MASTER_2 = 2
 *
 *                  hal_i2c_frequency_t frequency
 *                              HAL_I2C_FREQUENCY_50K  = 0,
 *                              HAL_I2C_FREQUENCY_100K = 1,
 *                              HAL_I2C_FREQUENCY_200K = 2,
 *                              HAL_I2C_FREQUENCY_400K = 3,
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
uint8_t senddata[1]={0x7f};//8374 ID REG
uint8_t receivedata[1]={0};
uint8_t sendregdata[3]={0x38,0x1D,0x1E};//8374 ID REG
uint8_t receiveregdata[3]={0,0,0};
CODEC_STATUS_e es8374_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency)
{
	/*I2C*/
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;
    /*
    hal_i2c_send_to_receive_config_t  i2c_send_to_receive_config;
    i2c_send_to_receive_config.slave_address= ES8374_I2C_ADDR ;
    i2c_send_to_receive_config.send_data= senddata ;
    i2c_send_to_receive_config.send_length= 1 ;
    i2c_send_to_receive_config.receive_buffer= receivedata ;
    i2c_send_to_receive_config.receive_length= 1 ;

    hal_i2c_send_to_receive_config_t  i2c_send_to_receive_reg_config;
    i2c_send_to_receive_reg_config.slave_address= ES8374_I2C_ADDR ;
    i2c_send_to_receive_reg_config.send_data= sendregdata ;
    i2c_send_to_receive_reg_config.send_length= 1 ;
    i2c_send_to_receive_reg_config.receive_buffer= receiveregdata ;
    i2c_send_to_receive_reg_config.receive_length= 1 ;
    */
    hal_i2c_send_to_receive_config_t i2c_send_to_receive_config={ES8374_I2C_ADDR,senddata,1,receivedata,1};
    hal_i2c_send_to_receive_config_t i2c_send_to_receive_reg_config={ES8374_I2C_ADDR,sendregdata,1,receiveregdata,1};

    i2c_status = HAL_I2C_STATUS_OK;
    i2c_config.frequency = (hal_i2c_frequency_t)frequency;
    es8374_i2c_port = (hal_i2c_port_t)i2c_port;

    i2c_status = hal_i2c_master_init(es8374_i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
    	printf("[es8374] i2c init ok\r\n");
    } else {
    	printf("[es8374] i2c init error\r\n");
    }
    hal_gpt_delay_ms(10);
    /////////////////////////////////////////////////////////////////
    //Read es8374id.
    printf("[es8374] read es8374 id\r\n");
	if (HAL_I2C_STATUS_OK !=hal_i2c_master_send_to_receive_polling(es8374_i2c_port,&i2c_send_to_receive_config))
    {
       	printf("[es8374] i2c write  0x7f error\r\n");
    }else
    {
        printf("[es8374] i2c write  0x7f ok\r\n");
        printf("[es8374] es8374 id=0x%02x\r\n"  ,receivedata[0] );
    }
    hal_gpt_delay_ms(10);
    /////////////////////////////////////////////////////////////////
    if(receivedata[0]==es8374_id)
    {
		if(es8374_init()==1)
		{
			printf("[es8374] chip write error\r\n");
		}
		else
		{
			printf("[es8374] chip write ok\r\n");
		}
		hal_gpt_delay_ms(10);
    }
    /////////////////////////////////////////////////////////////////
    //Read es8374reg.
  	if (HAL_I2C_STATUS_OK !=hal_i2c_master_send_to_receive_polling(es8374_i2c_port,&i2c_send_to_receive_reg_config))
    {
        printf("[es8374] i2c read regvalue error\r\n");
    }else
    {
        printf("[es8374] i2c read regvalue ok\r\n");
        printf("[es8374] 0x38 =0x%02x\r\n"  ,receiveregdata[0]);
    }
    hal_gpt_delay_ms(10);
    /*
    if (HAL_I2C_STATUS_OK != hal_i2c_master_deinit(es8374_i2c_port)) {
         printf("I2C deinit step2 error\r\n");
    }
    else
    {
      	 printf("I2C deinit step2 ok\r\n");
    }
    */
    hal_gpt_delay_ms(200);
    return CODEC_STATUS_OK;
}
/*****************************************************************************************************************
 * @Function:
 *      es8374_i2c_deinit
 *
 * @Routine Description:
 *      i2c deinitial
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/

CODEC_STATUS_e es8374_i2c_deinit()
{
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_status = hal_i2c_master_deinit(es8374_i2c_port);

    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8374]i2c deinit ok\n");
    } else {
        log_hal_error("[es8374]i2c deinit error\n");
        return CODEC_STATUS_ERROR;
    }
    return CODEC_STATUS_OK;
}

