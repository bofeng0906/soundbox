#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_log.h"
#include "hal_pinmux_define.h"
#include <stdio.h>
#include <string.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// abstarct interface
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mt768x_i2c.h"
//#define ES8316_REG_SHIFT
//#define ES8316_REG_READ_CHECK
#define ES8316_I2C_ADDR                               0x10
#define USER_VOLUME_LEVEL 100
static hal_i2c_port_t es8316_i2c_port;
static unsigned char g_IotVolSPKValue = 60;

static unsigned short es8316_i2c_read(
        unsigned short u16Addr)
{
    unsigned char RegAddress;
    unsigned char u8TxDataBuf[2];
    unsigned short u16Data;
    int codec_status;
    hal_i2c_status_t i2c_status;

    RegAddress = (unsigned char)u16Addr;
    memset(u8TxDataBuf, 0, 2);

    u16Data = 0;
    codec_status = 0;
    i2c_status = HAL_I2C_STATUS_OK;

#ifdef ES8316_REG_SHIFT
    RegAddress = RegAddress << 1 | 0x0;
#endif

    i2c_status = hal_i2c_master_send_polling(es8316_i2c_port, ES8316_I2C_ADDR, &RegAddress, 1);
    if (i2c_status == HAL_I2C_STATUS_OK) {
    } else {
        codec_status = -1;
        log_hal_error("[es8316] aucodec_i2c_read prepare fail: RegAddress=%02x\n", (unsigned char)u16Addr);
        return codec_status;
    }

    //i2c_status = hal_i2c_master_receive_polling(es8316_i2c_port, ES8316_I2C_ADDR, u8TxDataBuf, 2);
    i2c_status = hal_i2c_master_receive_polling(es8316_i2c_port, ES8316_I2C_ADDR, u8TxDataBuf, 1);
    if (i2c_status == HAL_I2C_STATUS_OK) {
        u16Data = u8TxDataBuf[0];
		
		printf("[es8316] 6987 aucodec_i2c_read : u16Data=%02x\n", u16Data);
    } else {
        codec_status = -1;
        log_hal_error("[es8316] aucodec_i2c_read fail: RegAddress=%02x\n", (unsigned char)u16Addr);
    }

    return u16Data;

}

static int es8316_i2c_write(
    unsigned short u16Addr,
    unsigned short u16Data)
{
    unsigned short u16Temp;
    unsigned char u8TxDataBuf[2];

    hal_i2c_status_t i2c_status;

    int codec_status = 0;
    i2c_status = HAL_I2C_STATUS_OK;

    memset(u8TxDataBuf, 0, 2);


        //es8316_reg_cache[u16Addr] = u16Data;  // sync registers cache

        //log_hal_info("[es8316]           write: RegAddress:0x%02x, Data:0x%02x\n", u16Addr, u16Data);
#ifdef ES8316_REG_SHIFT
        u16Temp = u16Data & 0x01ff;
        u16Temp = u16Temp >> 8;
        u16Addr = (u16Addr << 1 | u16Temp);
#endif
        u8TxDataBuf[0] = (unsigned char)u16Addr;
        u8TxDataBuf[1] = (unsigned char)u16Data;

        i2c_status = hal_i2c_master_send_polling(es8316_i2c_port, ES8316_I2C_ADDR, u8TxDataBuf, 2);
        if (i2c_status == HAL_I2C_STATUS_OK) {
			
			printf("[es8316] 6987 aucodec_i2c_write success: u8TxDataBuf0=%02x, u8TxDataBuf1=%02x\n", u8TxDataBuf[0],u8TxDataBuf[1]);
#ifdef ES8316_REG_READ_CHECK
            log_hal_error("==============================================> i2c read back: 0x%02x:0x%02x\n",u16Addr, es8316_i2c_read(u16Addr));
#endif
        } else {
            codec_status = -1;
			
			printf("[es8316] 6987 aucodec_i2c_writef fail : u8TxDataBuf=%02x, u8TxDataBuf1=%02x\n", u8TxDataBuf[0],u8TxDataBuf[1]);
            log_hal_error("[es8316]   aucodec_i2c_write fail: RegAddress=%02x  Data=%u\n", (unsigned char)u8TxDataBuf[0], (unsigned char)u8TxDataBuf[1]);
        }

        hal_gpt_delay_ms(5);

    return codec_status;
}


// no noise config
const uint8_t ES8316_All_InitData[][4] =
{
    /*nbytes,  register,  data*/
    { 2, 0x00, 0x3F, 0 }, //IC Rst On
    { 50, 0x00, 0x00, 0 },
    { 2, 0x00, 0x00, 0 }, //IC Normal
    { 2, 0x0C, 0xFF, 0 }, // Cal Time Set

    { 2, 0x09, 0x04, 0 }, //Cpclk Set
    { 2, 0x02, 0x09, 0 }, //Clock Set
    { 2, 0x03, 0x20, 0 }, //adc Clock Set
    { 2, 0x04, 0x11, 0 }, //ADC Clock Set		//512RATIO
    { 2, 0x05, 0x00, 0 },
    { 2, 0x06, 0x11, 0 }, //dac Clock Set		//512RATIO
    { 2, 0x07, 0x00, 0 }, //dac Clock Set
    { 2, 0x08, 0x00, 0 }, //Bclk Set
    { 2, 0x01, 0x7F, 0 }, //:System Clock Set

    { 2, 0x1C, 0x0F, 0 }, //:CAL set
    { 2, 0x1E, 0x90, 0 }, //CAL set
    { 2, 0x1F, 0x90, 0 }, //:CAL set
    { 2, 0x27, 0x00, 0 },
    { 2, 0x22, 0x20, 0 },
    { 2, 0x24, 0x01, 0 },
    { 2, 0x25, 0x08, 0 },

    { 2, 0x30, 0x10, 0 },
    { 2, 0x31, 0x20, 0 }, //dac Single Speed
    { 2, 0x32, 0x00, 0 }, //mono (L+R)/2 to DACL and ACLR
    { 2, 0x33, 0x00, 0 }, //dacVolumeL set
    { 2, 0x34, 0x00, 0 }, //dacVolumeR set
    { 2, 0x0A, 0x0C, 0 },//16BIT=LJ
    { 2, 0x0B, 0x0C, 0 }, //:dac IIS Mode		//16BIT-LJ

    { 2, 0x10, 0x11, 0 }, //:dac set
    { 2, 0x11, 0xFC, 0 },
    { 2, 0x12, 0x28, 0 },
    { 2, 0x0E, 0x04, 0 }, //:Lower Power Mode
    { 2, 0x0F, 0x0C, 0 }, //:Lower Power Mode

    { 2, 0x2F, 0x00, 0 }, //:dac set
    { 2, 0x13, 0x00, 0 }, //:Hpmix Set
    { 2, 0x14, 0x88, 0 }, //:Hpmix Set
    { 2, 0x15, 0x88, 0 }, //:Hpmix Set
    { 2, 0x16, 0xBB, 0 }, //:Hpmix Set
    { 2, 0x1A, 0x10, 0 }, //:Cphp Set
    { 2, 0x1B, 0x30, 0 }, //:Cphp Set
    { 2, 0x19, 0x02, 0 }, //:Cphp Set
    { 2, 0x18, 0x00, 0 }, //:Cphp Set

    { 2, 0x4D, 0x00, 0 },
    { 2, 0x4E, 0x02, 0 },//PGA����21DB
    { 2, 0x50, 0xA0, 0 },
    { 2, 0x51, 0x00, 0 },
    { 2, 0x52, 0x04, 0 },
    { 2, 0x0D, 0x00, 0 },

#ifdef ES8316_ENABLE_EQ
    { 2, 0x35, 0x01, 0 },
    { 2, 0x36, 0x43, 0 },
    { 2, 0x37, 0x5D, 0 },
    { 2, 0x38, 0xFF, 0 },
    { 2, 0x39, 0x20, 0 },
    { 2, 0x3A, 0x3A, 0 },
    { 2, 0x3B, 0x1F, 0 },
    { 2, 0x3C, 0x2B, 0 },
    { 2, 0x3D, 0x01, 0 },
    { 2, 0x3E, 0x22, 0 },
    { 2, 0x3F, 0xA5, 0 },
    { 2, 0x40, 0x5C, 0 },
    { 2, 0x41, 0x23, 0 },
    { 2, 0x42, 0x22, 0 },
    { 2, 0x43, 0x5D, 0 },
    { 2, 0x44, 0x7E, 0 },
    { 2, 0x45, 0x05, 0 },
    { 2, 0x46, 0x56, 0 },
    { 2, 0x47, 0x75, 0 },
    { 2, 0x48, 0xFF, 0 },
    { 2, 0x49, 0x23, 0 },
    { 2, 0x4A, 0x44, 0 },
    { 2, 0x4B, 0xF1, 0 },
    { 2, 0x4C, 0xBE, 0 },

#endif

    // for record

    { 2, 0x23, 0x30, 0 },
    { 2, 0x24, 0x01, 0 },
    { 2, 0x25, 0x08, 0 },
    { 2, 0x29, 0xCD, 0 },
    { 2, 0x2A, 0x08, 0 },
    { 2, 0x2B, 0xA0, 0 },
    { 2, 0x2C, 0x05, 0 },
    { 2, 0x2D, 0x06, 0 },
    { 2, 0x2E, 0x61, 0 },


    // for end
    { 2, 0x00, 0xC0, 0 },
    { 50, 0x00, 0x00, 0 },
    { 2, 0x17, 0x66, 0 },

    { 0xFF, 0xFF, 0xFF, 0xFF }
};

static int reInit(const uint8_t initdata[][4])
{
    unsigned char nbytes;
    unsigned char i = 0, j = 0;
    unsigned char writeData[4];
    unsigned char *p = (unsigned char *)writeData;
    while (*p != 0xff)
    {
        for (i = 0; i < 4; i++)
        {
            writeData[i] = initdata[j][i];
        }

        if (*p == 0xff)
            break;

        nbytes = *p;
        if (nbytes > 10)
        {
            vTaskDelay(nbytes);
        }
        else
        {
			unsigned short a, d;
			a = *(p +1);
			d = *(p +2);
            es8316_i2c_write(a, d);
            vTaskDelay(2);
        }


        j++;
    }
	return 0;
}


#define MODE_10  10

int mt768x_audio_i2c_init(void)
{
    /* I2C */
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_config.frequency = (hal_i2c_frequency_t)HAL_I2C_FREQUENCY_50K;
    es8316_i2c_port = (hal_i2c_port_t)HAL_I2C_MASTER_0;

    i2c_status = hal_i2c_master_init(es8316_i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8316] mt768x_audio_i2c_init ok\n");
    } else {
        log_hal_error("[es8316] mt768x_audio_i2c_init error\n");
        return -1;
    }

    return reInit(ES8316_All_InitData);
}

int mt768x_audio_i2c_deinit(void)
{
    int codec_status;
    hal_i2c_status_t i2c_status;

    codec_status = 0;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_status = hal_i2c_master_deinit(es8316_i2c_port);

    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_error("[es8316] mt768x_audio_i2c_deinit ok\n");
    } else {
        codec_status = -1;
        log_hal_error("[es8316] mt768x_audio_i2c_deinit error\n");

    }

    return codec_status;
}

int mt768x_audio_i2c_set_parameters(unsigned char isinput, unsigned short samplerate, unsigned char channelnum)
{
    return 0;
}


const static uint8_t g_VolumeTable[101] =
        {
                192,96, 95, 94, 93, 92, 91, 90, 89, 88,
                87, 86, 85, 84, 83, 82, 81, 80, 79, 78,
                77, 76, 75, 74, 73, 72, 71, 70, 69, 68,
                67, 66, 65, 64, 63, 62, 61, 60, 59, 58,
                57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
                47, 46, 45, 44, 43, 42, 41, 40, 39, 38,
                37, 36, 35, 34, 33, 32, 31, 30, 29, 28,
                27, 26, 25, 24, 23, 22, 21, 20, 19, 18,
                17, 16, 15, 14, 13, 12, 11, 10,  9,  8,
                7,  8,  6,  5,  4,  3,  2,  1,  0,  0,
                0
        };

void mt768x_audio_i2c_set_mute(char enable)
{

}
void mt768x_audio_i2c_set_volume(unsigned char volume)
{
    if (volume > USER_VOLUME_LEVEL)
        volume = USER_VOLUME_LEVEL;
    if(volume < 0)
        volume = 0;
    g_IotVolSPKValue = volume;

    OAL_printf("[audio_i2c_es8316]: Set speaker volume: %d \r\n", g_VolumeTable[volume]);

    // left volume
    es8316_i2c_write(0x33, g_VolumeTable[volume]);
    vTaskDelay(2);
    es8316_i2c_write(0x34, g_VolumeTable[volume]);
    vTaskDelay(2);
}
unsigned char mt768x_audio_i2c_get_volume()
{
    return g_IotVolSPKValue;
}
void mt768x_audio_i2c_set_input(MT768X_AUDIO_INPUT_MODE mode)
{

}

void mt768x_audio_i2c_set_output(MT768X_AUDIO_OUTPUT_MODE mode)
{

}

