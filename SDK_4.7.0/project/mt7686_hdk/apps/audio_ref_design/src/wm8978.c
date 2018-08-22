/*
 * wm8978.c
 *
 *  Created on: 2018年5月28日
 *      Author: ydg
 */

#include "hal_gpt.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_pinmux_define.h"
#include "memory_attribute.h"
#include "hal.h"
#include "hal_platform.h"
#include "wm8978.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

hal_i2c_port_t wm8978_i2c_port;
static uint16_t WM8978_REGVAL[58]=
{
	0X0000,0X0000,0X0000,0X0000,0X0050,0X0000,0X0140,0X0000,
	0X0000,0X0000,0X0000,0X00FF,0X00FF,0X0000,0X0100,0X00FF,
	0X00FF,0X0000,0X012C,0X002C,0X002C,0X002C,0X002C,0X0000,
	0X0032,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,
	0X0038,0X000B,0X0032,0X0000,0X0008,0X000C,0X0093,0X00E9,
	0X0000,0X0000,0X0000,0X0000,0X0003,0X0010,0X0010,0X0100,
	0X0100,0X0002,0X0001,0X0001,0X0039,0X0039,0X0039,0X0039,
	0X0001,0X0001
};

//WM8978 DAC/ADC配置
//adcen:adc使能(1)/关闭(0)
//dacen:dac使能(1)/关闭(0)
void WM8978_ADDA_Cfg(uint8_t dacen,uint8_t adcen)
{
	uint16_t regval;
	regval=wm8978_i2c_read(3);							//读取R3
	if(dacen)
		regval|=3<<0;									//R3最低2个位设置为1,开启DACR&DACL
	else
		regval&=~(3<<0);								//R3最低2个位清零,关闭DACR&DACL.
	wm8978_i2c_write(3,regval);							//设置R3
	regval=wm8978_i2c_read(2);							//读取R2
	if(adcen)
		regval|=3<<0;			        				//R2最低2个位设置为1,开启ADCR&ADCL
	else
		regval&=~(3<<0);				  				//R2最低2个位清零,关闭ADCR&ADCL.
	wm8978_i2c_write(2,regval);							//设置R2
}
//WM8978 输入通道配置
//micen:MIC开启(1)/关闭(0)
//lineinen:Line In开启(1)/关闭(0)
//auxen:aux开启(1)/关闭(0)
void WM8978_Input_Cfg(uint8_t micen,uint8_t lineinen,uint8_t auxen)
{
	uint16_t regval;
	regval=wm8978_i2c_read(2);							    //读取R2
	if(micen)
		regval|=3<<2;										//开启INPPGAENR,INPPGAENL(MIC的PGA放大)
	else
		regval&=~(3<<2);									//关闭INPPGAENR,INPPGAENL.
	wm8978_i2c_write(2,regval);							    //设置R2
	regval=wm8978_i2c_read(44);							    //读取R44
	if(micen)
		regval|=3<<4|3<<0;									//开启LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	else
		regval&=~(3<<4|3<<0);								//关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
	wm8978_i2c_write(44,regval);						    //设置R44
	if(lineinen)
		WM8978_LINEIN_Gain(5);								//LINE IN 0dB增益
	else
		WM8978_LINEIN_Gain(0);								//关闭LINE IN
	if(auxen)
		WM8978_AUX_Gain(7);									//AUX 6dB增益
	else
		WM8978_AUX_Gain(0);									//关闭AUX输入
}
//WM8978 MIC增益设置(不包括BOOST的20dB,MIC-->ADC输入部分的增益)
//gain:0~63,对应-12dB~35.25dB,0.75dB/Step
void WM8978_MIC_Gain(uint8_t gain)
{
	gain&=0X3F;
	wm8978_i2c_write(45,gain);						//R45,左通道PGA设置
	wm8978_i2c_write(46,gain|1<<8);					//R46,右通道PGA设置
}
//WM8978 L2/R2(也就是Line In)增益设置(L2/R2-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void WM8978_LINEIN_Gain(uint8_t gain)
{
	uint16_t regval;
	gain&=0X07;
	regval=wm8978_i2c_read(47);					//读取R47
	regval&=~(7<<4);							//清除原来的设置
	wm8978_i2c_write(47,regval|gain<<4);		//设置R47
	regval=wm8978_i2c_read(48);					//读取R48
	regval&=~(7<<4);							//清除原来的设置
	wm8978_i2c_write(48,regval|gain<<4);		//设置R48
}
//WM8978 AUXR,AUXL(PWM音频部分)增益设置(AUXR/L-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void WM8978_AUX_Gain(uint8_t gain)
{
	uint16_t regval;
	gain&=0X07;
	regval=wm8978_i2c_read(47);					//读取R47
	regval&=~(7<<0);							//清除原来的设置
	wm8978_i2c_write(47,regval|gain<<0);		//设置R47
	regval=wm8978_i2c_read(48);					//读取R48
	regval&=~(7<<0);							//清除原来的设置
	wm8978_i2c_write(48,regval|gain<<0);		//设置R48
}

//WM8978 输出配置
//dacen:DAC输出(放音)开启(1)/关闭(0)
//bpsen:Bypass输出(录音,包括MIC,LINE IN,AUX等)开启(1)/关闭(0)
void WM8978_Output_Cfg(uint8_t dacen,uint8_t bpsen)
{
	uint16_t regval=0;
	if(dacen)
		regval|=1<<0;									//DAC输出使能
	if(bpsen)
	{
		regval|=1<<1;									//BYPASS使能
		regval|=5<<2;									//0dB增益
	}
	wm8978_i2c_write(50,regval);						//R50设置
	wm8978_i2c_write(51,regval);						//R51设置
}
//设置耳机左右声道音量
//voll:左声道音量(0~63)
//volr:右声道音量(0~63)
void WM8978_HPvol_Set(uint8_t voll,uint8_t volr)
{
	voll&=0X3F;
	volr&=0X3F;										//限定范围
	if(voll==0)voll|=1<<6;							//音量为0时,直接mute
	if(volr==0)volr|=1<<6;							//音量为0时,直接mute
	wm8978_i2c_write(52,voll);						//R52,耳机左声道音量设置
	wm8978_i2c_write(53,volr|(1<<8));				//R53,耳机右声道音量设置,同步更新(HPVU=1)
}
//设置喇叭音量
//voll:左声道音量(0~63)
void WM8978_SPKvol_Set(uint8_t volx)
{
	volx&=0X3F;//限定范围
	if(volx==0)volx|=1<<6;						    //音量为0时,直接mute
	wm8978_i2c_write(54,volx);					    //R54,喇叭左声道音量设置
	wm8978_i2c_write(55,volx|(1<<8));				//R55,喇叭右声道音量设置,同步更新(SPKVU=1)
}
//设置I2S工作模式
//fmt:0,LSB(右对齐);1,MSB(左对齐);2,飞利浦标准I2S;3,PCM/DSP;
//len:0,16位;1,20位;2,24位;3,32位;
void WM8978_I2S_Cfg(uint8_t fmt,uint8_t len)
{
	fmt&=0x03;
	len&=0x03;															//限定范围
	wm8978_i2c_write(4,(fmt<<3)|(len<<5));	//R4,WM8978工作模式设置
}
uint16_t wm8978_i2c_read(uint8_t reg)
{
	return WM8978_REGVAL[reg];
}
/////////////////////////////////////////////////////////////////////////////////
uint8_t wm8978_i2c_write(uint8_t u8Addr,uint16_t u8Data)
{
    uint8_t u8TxDataBuf[2];
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    u8TxDataBuf[0] = (u8Addr<<1)|((uint8_t)((u8Data>>8)&0x01));
    u8TxDataBuf[1] = (uint8_t )u8Data;
    hal_gpt_delay_ms(10);
    i2c_status = hal_i2c_master_send_polling(wm8978_i2c_port, WM8978_I2C_ADDR, u8TxDataBuf, 2);
    if (i2c_status != HAL_I2C_STATUS_OK)
    {
        printf("wm8978 i2c write fail: RegAddress=%02x  Data=%02x \r\n", u8TxDataBuf[0], u8TxDataBuf[1]);
	    return 1;
    }
    if (i2c_status == HAL_I2C_STATUS_OK)
    {
    	printf("wm8978 i2c write ok: RegAddress=%02x  Data=%02x \r\n", u8TxDataBuf[0], u8TxDataBuf[1]);
    	WM8978_REGVAL[u8Addr]=u8Data;
    	return 0;
    }
    return 0;
}

uint8_t wm8978_init(void)
{
	//I2C总线必须在调用该函数前完成初始化
	uint8_t Res;
	Res=wm8978_i2c_write(0,0);							//软复位WM8978
	if(Res)
		return 1;										//发送指令失败,WM8978异常
	wm8978_i2c_write(1,0X1B);							//R1,MICEN设置为1(MIC使能),BIASEN设置为1(模拟器工作),VMIDSEL[1:0]设置为:11(5K)
	wm8978_i2c_write(2,0X1B0);							//R2,ROUT1,LOUT1输出使能(耳机可以工作),BOOSTENR,BOOSTENL使能
	wm8978_i2c_write(3,0X6C);							//R3,LOUT2,ROUT2输出使能(喇叭工作),RMIX,LMIX使能
	wm8978_i2c_write(6,0);								//R6,MCLK由外部提供
	wm8978_i2c_write(43,1<<4);							//R43,INVROUT2反向,驱动喇叭
	wm8978_i2c_write(47,1<<8);							//R47设置,PGABOOSTL,左通道MIC获得20倍增益
	wm8978_i2c_write(48,1<<8);							//R48设置,PGABOOSTR,右通道MIC获得20倍增益
	wm8978_i2c_write(49,1<<1);							//R49,TSDEN,开启过热保护
	wm8978_i2c_write(10,1<<3);							//R10,SOFTMUTE关闭,128x采样,最佳SNR
	wm8978_i2c_write(14,1<<3);							//R14,ADC 128x采样率

	WM8978_I2S_Cfg(2,0);								//设置I2S接口模式，数据位数不需要设置，播放从设备不使用
	//放音设置
	WM8978_ADDA_Cfg(1,0);								//开启DAC
	WM8978_Input_Cfg(0,0,0);						    //关闭输入通道
	WM8978_Output_Cfg(1,0);							    //开启DAC输出
	//录音设置
	//WM8978_ADDA_Cfg(0,1);							    //开启ADC
	//WM8978_Input_Cfg(1,1,0);						    //开启输入通道(MIC&LINE IN)
	//WM8978_Output_Cfg(0,1);							//开启BYPASS输出
	//WM8978_MIC_Gain(46);							    //MIC增益设置
	//////////////////////////
	//WM8978_HPvol_Set(30,30);
	WM8978_SPKvol_Set(40);//50

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t wm8978senddata[1]={0x03};//8978 reg
uint8_t wm8978recdata[1]={0};
WM8978_CODEC_STATUS_e wm8978_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency)
{
	/*I2C*/
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;
    /*
    hal_i2c_send_to_receive_config_t  i2c_send_to_receive_config;
    i2c_send_to_receive_config.slave_address= WM8978_I2C_ADDR ;
    i2c_send_to_receive_config.send_data= wm8978senddata ;
    i2c_send_to_receive_config.send_length= 1 ;
    i2c_send_to_receive_config.receive_buffer= wm8978recdata ;
    i2c_send_to_receive_config.receive_length= 1 ;
    */
    hal_i2c_send_to_receive_config_t i2c_send_to_receive_config={WM8978_I2C_ADDR,wm8978senddata,1,wm8978recdata,1};

    i2c_status = HAL_I2C_STATUS_OK;
    i2c_config.frequency = (hal_i2c_frequency_t)frequency;
    wm8978_i2c_port = (hal_i2c_port_t)i2c_port;

    i2c_status = hal_i2c_master_init(wm8978_i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
    	printf("[wm8978] i2c init ok\r\n");
    } else {
    	printf("[wm8978] i2c init error\r\n");
    }
    hal_gpt_delay_ms(10);
    /////////////////////////////////////////////////////////////////
    /*
    //Read wm8978 id.
    printf("[wm8978] read wm8978 reg\r\n");
	if (HAL_I2C_STATUS_OK !=hal_i2c_master_send_to_receive_polling(wm8978_i2c_port,&i2c_send_to_receive_config))
    {
       	printf("[wm8978] i2c read  0x03 error\r\n");
    }else
    {
        printf("[wm8978] i2c read  0x03 ok\r\n");
        printf("[wm8978] wm8978 reg=0x%02x\r\n"  ,wm8978recdata[0] );
    }
    hal_gpt_delay_ms(10);
    */
    /////////////////////////////////////////////////////////////////
    //if(wm8978recdata[0]==wm8978_id)
    //{
		if(wm8978_init()==1)
		{
			printf("[wm8978] chip write error\r\n");
		}
		else
		{
			printf("[wm8978] chip write ok\r\n");
		}
		hal_gpt_delay_ms(10);
    //}
    /////////////////////////////////////////////////////////////////
	/*
    //Read wm8978 reg.
    printf("[wm8978] read wm8978 reg\r\n");
	if (HAL_I2C_STATUS_OK !=hal_i2c_master_send_to_receive_polling(wm8978_i2c_port,&i2c_send_to_receive_config))
	{
		printf("[wm8978] i2c read  0x03 error\r\n");
	}else
	{
		printf("[wm8978] i2c read  0x03 ok\r\n");
		printf("[wm8978] wm8978 reg=0x%02x\r\n"  ,wm8978recdata[0] );
	}
	hal_gpt_delay_ms(200);
	*/
    /*
    if (HAL_I2C_STATUS_OK != hal_i2c_master_deinit(wm8978_i2c_port)) {
         printf("I2C deinit step2 error\r\n");
    }
    else
    {
      	 printf("I2C deinit step2 ok\r\n");
    }
    */
    hal_gpt_delay_ms(200);
    return WM8978_CODEC_STATUS_OK;
}
/*****************************************************************************************************************
 * @Function:
 *      wm8978_i2c_deinit
 *
 * @Routine Description:
 *      i2c deinitial
 *
 * @Parameter:
 *
 * @Return:
 *      WM8978_AUCODEC_STATUS_OK:    Success
 *      WM8978_AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/

WM8978_CODEC_STATUS_e wm8978_i2c_deinit()
{
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_status = hal_i2c_master_deinit(wm8978_i2c_port);

    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[wm8978]i2c deinit ok\n");
    } else {
        log_hal_error("[wm8978]i2c deinit error\n");
        return WM8978_CODEC_STATUS_ERROR;
    }
    return WM8978_CODEC_STATUS_OK;
}
