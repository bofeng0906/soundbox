/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */


#include "hal_gpt.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_log.h"
#include "hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "es8374.h"
#include <stdio.h>
#include <string.h>
#include "mt768x_i2c.h"

#define VOLUME_MAX		(10)

//extern SemaphoreHandle_t semaphore_i2c_port;
static hal_i2c_port_t es8374_i2c_port;
static uint8_t es8374_init = 0;
static uint8_t volume_value = VOLUME_MAX;

uint8_t aucodec_volume_reg(uint8_t volume)
{
	uint8_t vol_reg;

	if(volume == 10)
		vol_reg = 0;
	else if(volume == 9)
		vol_reg = 5;
	else if(volume == 8)
		vol_reg = 10;
	else if(volume == 7)
		vol_reg = 15;
	else if(volume == 6)
		vol_reg = 20;
	else if(volume == 5)
		vol_reg = 25;
	else if(volume == 4)
		vol_reg = 30;
	else if(volume == 3)
		vol_reg = 35;
	else if(volume == 2)
		vol_reg = 40;
	else if(volume == 1)
		vol_reg = 45;
	else if(volume == 0)
		vol_reg = 50;

	return vol_reg;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_write
 *
 * @Routine Description:
 *      Audio codec write register
 *
 * @Parameter:
 *      u16Addr:    Register address
 *      u16Data:    Register data
 *
 * @Return:
 *      1:    Success
 *      -1:    Failure
 *
 *****************************************************************************************************************/
int8_t aucodec_i2c_write(
    uint8_t u8Addr,
    uint8_t u8Data)
{
	uint8_t u8TxDataBuf[2];

	hal_i2c_status_t i2c_status;
	i2c_status = HAL_I2C_STATUS_OK;

	u8TxDataBuf[0] = u8Addr;
	u8TxDataBuf[1] = u8Data;

	i2c_status = hal_i2c_master_send_polling(es8374_i2c_port, ES8374_I2C_ADDR, u8TxDataBuf, 2);
	if (i2c_status != HAL_I2C_STATUS_OK) {
	    log_hal_error("[es8374]i2c write fail: RegAddress=%02x  Data=%u\n\r", u8TxDataBuf[0], u8TxDataBuf[1]);
	    return -1;
	}

	return 0;
}


/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_read
 *
 * @Routine Description:
 *      Audio Codec read register
 *
 * @Parameter:
 *      (in) u16Addr:    Register address
 *      (out) u16Data:  Data
 *
 * @Return:
 *      1:    Success
 *      -1:    Failure
 *
 *****************************************************************************************************************/
uint8_t aucodec_i2c_read(uint8_t u8Addr)
{
    uint8_t u8TxDataBuf;

    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;


    i2c_status = hal_i2c_master_send_polling(es8374_i2c_port, ES8374_I2C_ADDR, &u8Addr, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[es8374][1]i2c read fail1: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    i2c_status = hal_i2c_master_receive_polling(es8374_i2c_port, ES8374_I2C_ADDR, &u8TxDataBuf, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[es8374][2]i2c read fail2: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    return u8TxDataBuf;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_init
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
AUCODEC_STATUS_e aucodec_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency)
{
    /*I2C*/
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;

    //xSemaphoreTake(semaphore_i2c_port, portMAX_DELAY);

    i2c_status = HAL_I2C_STATUS_OK;
    i2c_config.frequency = (hal_i2c_frequency_t)frequency;
    es8374_i2c_port = (hal_i2c_port_t)i2c_port;

    i2c_status = hal_i2c_master_init(es8374_i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8374]i2c init ok\n");
    } else {
        log_hal_error("[es8374]i2c init error\n");
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_deinit
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

AUCODEC_STATUS_e aucodec_i2c_deinit()
{
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_status = hal_i2c_master_deinit(es8374_i2c_port);

    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8374]i2c deinit ok\n");
    } else {
        log_hal_error("[es8374]i2c deinit error\n");
        return AUCODEC_STATUS_ERROR;
    }

    //xSemaphoreGive(semaphore_i2c_port);

    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_output
 *
 * @Routine Description:
 *      Audio codec set analog output path for DACIN
 *
 * @Parameter:
 *      AUCODEC_OUTPUT_SEL_e:    eAuxOut
 *                               eHpOut
 *                               eSpkOut
 *                               eNoneOut
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_output(
    AUCODEC_OUTPUT_SEL_e eOutputMode)
{
    int8_t ret = 0;
    uint16_t u16Data;

    if ((eOutputMode == eLineOut) || (eOutputMode == eSpkOut)) {


    } else {

    }

    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_input
 *
 * @Routine Description:
 *      Audio codec set analog input path for ADCOUT
 *
 * @Parameter:
 *      AUCODEC_INPUT_SEL_e:    eAuxIn
 *                              eMicIn
 *                              eNoneIn
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_input(
    AUCODEC_INPUT_SEL_e eInputMode)
{
    int8_t ret = 0;
    uint16_t u16Data;

    if (eInputMode == eMicIn) {


    } else {

    }

    return AUCODEC_STATUS_OK;

}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_mute
 *
 * @Routine Description:
 *      Audio codec software reset
 *
 * @Parameter:
 *      AUCODEC_MUTE_e:    eMute
 *                         eUnmute
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_mute(
    AUCODEC_MUTE_e eMuteEnable)
{
    int8_t ret = 0;
    uint16_t u16Data;

    // DAC soft-mute control
    if (eMuteEnable == eMute) {
		aucodec_i2c_write(0x36, 0x20);
    } else {
		aucodec_i2c_write(0x36, 0x00);
    }

    return AUCODEC_STATUS_OK;
}


/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_dai_fmt
 *
 * @Routine Description:
 *      Audio codec digital audio interface format configuration
 *
 * @Parameter:
 *      AUCODEC_DAI_FORMAT_e:    eRightJustified
 *                               eLeftJustified
 *                               eI2S
 *                               ePCMA
 *                               ePCMB
 *      AUCODEC_DAI_WLEN_e:    e16Bit
 *                             e20Bit
 *                             e24Bit
 *                             e32Bit
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_dai_fmt(
    AUCODEC_DAI_FORMAT_e eFormat,
    AUCODEC_DAI_WLEN_e eWLEN,
    AUCODEC_BCLK_INV_e eBCLKP)
{
    int8_t ret = 0;
    uint16_t u16Data;

    // DAI format configuration
    switch (eFormat) {
        case eRightJustified:
            break;
        case eLeftJustified:
            break;
        case eI2S:
            break;
        case ePCMA:
            break;
        default: // case ePCMB:
            break;
    }

    // Data resolution of data stream
    switch (eWLEN) {
        case e16Bit:
            break;
        case e20Bit:
            break;
        case e24Bit:
            break;
        default:  // case e32Bit:
            break;
    }

    switch (eBCLKP) {
        case eBCLK_INV:
            break;
        default: //BCLK Polarity Normal
            break;
    }

    //ret = aucodec_i2c_write(AUCODEC_R04H_AU_INTERFACE_CTRL_REG, u16Data);
    if (ret == -1) {
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}


/*****************************************************************************************************************
* @Function:
*      aucodec_set_dai_sysclk
*
* @Routine Description:
*      Audio codec system clock configuration
*
* @Parameter:
*      AUCODEC_SAMPLERATE_SEL_e:    eSR48KHz
*                                   eSR44K1Hz
*                                   eSR32KHz
*                                   eSR16KHz
*                                   eSR8KHz
*      AUCODEC_DAI_ROLE_e:     eSLAVE
*                              eMASTER
*      AUCODEC_BCLK_RATE_e:     e256xFS
*                               e128xFS
*                               e64xFS
*                               e32xFS
*                               e16xFS
*                               e8xFS
*      u32MclkRate:    The frequency of mclk input (Hz)
*      AUCODEC_PLL_e:    ePLLDisable
*                        ePLLEnable
*
* @Return:
*      AUCODEC_STATUS_OK:    Success
*      AUCODEC_STATUS_ERROR:    Failure
*****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_dai_sysclk(
    AUCODEC_SAMPLERATE_SEL_e eSampleRate,
    AUCODEC_DAI_ROLE_e eDAIRole,
    AUCODEC_BCLK_RATE_e eBCLKRate,
    uint32_t u32MclkRate,
    AUCODEC_PLL_e ePLLCtrl)
{


    return AUCODEC_STATUS_OK;
}

void aucodec_set_sampling_rate(int bitrate)
{
	aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec

	switch (bitrate) {
		case 8000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   		

			break;
		case 11025:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x04); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);  

			break;
		case 12000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);  
			break;
		case 16000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0); 

			break;
		case 22050:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x02); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   	

			break;
		case 24000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   

			break;
		case 32000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   

			break;
		case 44100:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 512D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   

			break;
		case 48000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   

			break;
		case 96000:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);  
			break;
		default:
			aucodec_set_mute(1);
			aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 512D
			aucodec_i2c_write(0x07,0x00); 
			aucodec_set_mute(0);   

			log_hal_info("Unsupported sample rate, use default config 44.1kHz\n");
			break;
	}
	
	//aucodec_i2c_deinit();
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_softreset
 *
 * @Routine Description:
 *      Audio codec software reset
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_softreset(void)
{

	return 0;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_init
 *
 * @Routine Description:
 *      Audio codec initial
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_init(void)
{
	
    int8_t i,vol_reg = 0;
    uint16_t u16Data;
	

	if(es8374_init > 0)
		return 0;

	es8374_init = 1;
	vol_reg = aucodec_volume_reg(volume_value);

    printf("HAL_I2C_FREQUENCY_200K\n");
	
	aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
	
	aucodec_i2c_write(0x00,0x3F); //IC Rst start
	aucodec_i2c_write(0x00,0x03); //IC Rst stop
	aucodec_i2c_write(0x01,0xFF); //IC clk on
	
	aucodec_i2c_write(0x6F,0xA0); //pll set:mode enable
	aucodec_i2c_write(0x72,0x41); //pll set:mode set
	aucodec_i2c_write(0x09,0x01); //pll set:reset on ,set start
		/* PLL FOR 26MHZ/44.1KHZ */
	aucodec_i2c_write(0x0C,0x27); //pll set:k
	aucodec_i2c_write(0x0D,0xDC); //pll set:k
	aucodec_i2c_write(0x0E,0x2B); //pll set:k
	aucodec_i2c_write(0x0A,0x8A); //pll set:
	aucodec_i2c_write(0x0B,0x06);//pll set:n
	
	/* PLL FOR 26MHZ/48KHZ */
	/*
	aucodec_i2c_write(0x0C,0x17); //pll set:k
	aucodec_i2c_write(0x0D,0xA3); //pll set:k
	aucodec_i2c_write(0x0E,0x2F); //pll set:k
	aucodec_i2c_write(0x0A,0x8A); //pll set:
	aucodec_i2c_write(0x0B,0x07); //pll set:n
	*/	
	aucodec_i2c_write(0x09,0x41); //pll set:reset off ,set stop
	
	aucodec_i2c_write(0x05,0x11); //clk div =1
	aucodec_i2c_write(0x03,0x20); //osr =32
	aucodec_i2c_write(0x06,0x01); //LRCK div =0100H = 256D
	aucodec_i2c_write(0x07,0x00); 
	aucodec_i2c_write(0x0F,0x04); //MASTER MODE, BCLK = MCLK/4
	aucodec_i2c_write(0x10,0x0C); //I2S-16BIT, ADC
	aucodec_i2c_write(0x11,0x0C); //I2S-16BIT, DAC
	//aucodec_i2c_write(0x02,0x08);  //select PLL
	
	aucodec_i2c_write(0x24,0x08); //adc set
	aucodec_i2c_write(0x36,0x40); //dac set
	aucodec_i2c_write(0x12,0x30); //timming set
	aucodec_i2c_write(0x13,0x20); //timming set
	aucodec_i2c_write(0x21,0x50); //adc set: SEL LIN1 CH+PGAGAIN=0DB
	aucodec_i2c_write(0x22,0xff); //adc set: PGA GAIN=0DB
	aucodec_i2c_write(0x21,0x24); //adc set: SEL LIN1 CH+PGAGAIN=18DB
	aucodec_i2c_write(0x22,0x55); //pga = +15db
	aucodec_i2c_write(0x00,0x80); // IC START
	vTaskDelay(50);
	aucodec_i2c_write(0x14,0x8A); // IC START
	aucodec_i2c_write(0x15,0x40); // IC START
	aucodec_i2c_write(0x1A,0xA0); // monoout set
	aucodec_i2c_write(0x1B,0x19); // monoout set
	aucodec_i2c_write(0x1C,0x90); // spk set
	aucodec_i2c_write(0x1D,0x2b); // spk set
	aucodec_i2c_write(0x1F,0x00); // spk set
	aucodec_i2c_write(0x1E,0xA2); // spk on
	aucodec_i2c_write(0x28,0xa0); // alc set
	aucodec_i2c_write(0x26,0x50); 
	aucodec_i2c_write(0x25,0x00); // ADCVOLUME on
	aucodec_i2c_write(0x38,vol_reg); // DACVOLUMEL on
	aucodec_i2c_write(0x37,0x30); // dac set
	aucodec_i2c_write(0x6D,0x60); //SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT
	
	aucodec_i2c_write(0x71,0x05);
	aucodec_i2c_write(0x73,0x70);			
	aucodec_i2c_write(0x36,0x00); //dac set
	aucodec_i2c_write(0x37,0x00); // dac set 	

	//aucodec_i2c_deinit();

	log_hal_info("[es8374]aucodec_init ok\n");

	//aucodec_i2c_debug();

    return AUCODEC_STATUS_OK;
}

AUCODEC_STATUS_e aucodec_deinit(void)
{
	aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec

	aucodec_i2c_write(0x38,0xC0);
	aucodec_i2c_write(0x25,0xC0);
	aucodec_i2c_write(0x28,0x1C);
	aucodec_i2c_write(0x36,0x20);
	aucodec_i2c_write(0x37,0x01);
	aucodec_i2c_write(0x1A,0x08);
	aucodec_i2c_write(0x1D,0x10);
	aucodec_i2c_write(0x1E,0x40);
	aucodec_i2c_write(0x1C,0x10);
	aucodec_i2c_write(0x24,0x20);
	aucodec_i2c_write(0x22,0x00);
	aucodec_i2c_write(0x21,0xD4);
	aucodec_i2c_write(0x14,0x94);
	aucodec_i2c_write(0x15,0xBF);
	aucodec_i2c_write(0x09,0x80);
	aucodec_i2c_write(0x01,0xFF);
	aucodec_i2c_write(0x01,0x03);

	aucodec_i2c_deinit();

	es8374_init = 0;

	return AUCODEC_STATUS_OK;
}

AUCODEC_STATUS_e aucodec_set_volume(uint8_t volume)
{
	uint8_t vol_reg;

	vol_reg = aucodec_volume_reg(volume);

	aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec

	printf("aucodec_set_volume vol_reg=%d,\n", vol_reg);
	aucodec_i2c_write(0x38, vol_reg);

	aucodec_i2c_deinit();

	return AUCODEC_STATUS_OK;
}

void aucodec_volume_up(void)
{
	if(volume_value >= VOLUME_MAX)
		return;

	volume_value++;
	aucodec_set_volume(volume_value);
}

void aucodec_volume_down(void)
{
	if(volume_value <= 0)
		return;

	volume_value--;
	aucodec_set_volume(volume_value);
}

void aucodec_volume_init(void)
{
	nvram_volume_get(&volume_value);
}

void aucodec_volume_save(void)
{
	nvram_volume_set(volume_value);
}

void aucodec_i2c_debug(void)
{
	printf("\n\n");
	for(uint8_t i=0; i<=32; i++){
		printf("%d=0x%x,\n", i,aucodec_i2c_read(i));
	}
	printf("\n\n");
}

int mt768x_audio_i2c_set_parameters(unsigned char isinput, unsigned short samplerate, unsigned char channelnum)
{
    return 0;
}

void mt768x_audio_i2c_set_output(MT768X_AUDIO_OUTPUT_MODE mode)
{

}

int mt768x_audio_i2c_init(void)
{
	AUCODEC_STATUS_e init8374;
	init8374=aucodec_init();
	if(init8374==AUCODEC_STATUS_OK)
	{
		log_hal_info("[es8374] aucodec_init ok\n");
	}
	else
	{
		log_hal_info("[es8374] aucodec_init fail\n");
	}
    return init8374;
}