#include "memory_attribute.h"
#include "oal_audio.h"
#include "mt768x_i2c.h"
#include "hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define OAL_WAIT_FOREVER	0xFFFFFFFF
typedef void*					OAL_SEM;
#define MAXIMUM(a,b)            ((a) > (b) ? (a) : (b))
#define MINIMUM(a,b)            ((a) < (b) ? (a) : (b))

#ifndef HAL_I2S_TX_MONO_DUPLICATE_DISABLE
#define HAL_I2S_TX_MONO_DUPLICATE_DISABLE	0
#endif

OAL_SEM OAL_SemCreate(int initcount)
{
	OAL_SEM handle;

	handle = xSemaphoreCreateCounting(1, initcount);
	if (handle) {
		//sem_debug_add( (const xSemaphoreHandle)handle, "", 1);
		return handle;
	}
	else
		return 0;
}
int OAL_SemDestroy(OAL_SEM *sem)
{
	if (*sem){
		vSemaphoreDelete(*sem);
		//sem_debug_delete((const xSemaphoreHandle)*sem);
		*sem = NULL;
		return 0;
	}
	return -1;
}
int OAL_SemWait(OAL_SEM *sem)
{
	int ret;
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (!sem || !(*sem))
		return -2;
	if (__get_IPSR() != 0) {
		/* This call is from Cortex-M3 handler mode, i.e. exception
		 * context, hence use FromISR FreeRTOS APIs.
		 */
		ret = xSemaphoreTakeFromISR(*sem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	} else
		ret = xSemaphoreTake(*sem, OAL_WAIT_FOREVER);
	return ret == pdTRUE ? 0 : -1;
}
void OAL_SemPost(OAL_SEM *sem)
{
	int ret;
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (!sem || !(*sem))
		return;

	if (__get_IPSR() != 0) {
		/* This call is from Cortex-M3 handler mode, i.e. exception
		 * context, hence use FromISR FreeRTOS APIs.
		 */
		ret = xSemaphoreGiveFromISR(*sem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	} else
		ret = xSemaphoreGive(*sem);
}
void* OAL_calloc( unsigned int count, unsigned int size )
{
	void *ret;

	ret = pvPortMalloc(count*size);
	if (ret == 0) {
		log_hal_info("[oal_mem]: !!!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxoal_calloc1 %d failed.\r\n", size);
	}

	memset(ret, 0x00, size);
	return ret;
}
void OAL_free(void *memory)
{
	vPortFree(memory);
}
//#define ENABKE_TX_LOG
//#define ENABLE_RX_LOG
#define MT768X_I2S_INPUT_PORT  HAL_I2S_0
#define MT768X_I2S_OUTPUT_PORT HAL_I2S_0//HAL_I2S_1
#define MT768X_I2S_INPUT_PORT_HAS_I2C_CONFIG    0
#define MT768X_I2S_OUTPUT_PORT_HAS_I2C_CONFIG   1
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t i2s_tx_buffer[MAX_PCM_IO_LENGTH];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t i2s_rx_buffer[MAX_PCM_IO_LENGTH];
#define GPIO_ReadReg(addr)          (*(volatile uint32_t *)(addr))

typedef struct {
	OAL_AUDIO_CFG cfg;
	hal_i2s_port_t i2s_port;
	unsigned int remainNeedLength;
	OAL_SEM txWaitSem;
	OAL_SEM rxWaitSem;
} MT768X_AUDIO_HANDLE;

static hal_i2s_sample_rate_t sampleRate2HALIndex(unsigned int sampleRate)
{
	hal_i2s_sample_rate_t hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_48K;

	switch (sampleRate) {
	case 44100: // mp3 decoder SWIP = 44.1k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_44_1K;
		break;
	case 48000: // mp3 decoder SWIP = 48k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_48K;
		break;
	case 32000: // mp3 decoder SWIP = 32k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_32K;
		break;
	case 22050: // mp3 decoder SWIP = 22.05k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_22_05K;
		break;
	case 24000: // mp3 decoder SWIP = 24k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_24K;
		break;
	case 16000: // mp3 decoder SWIP = 16k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_16K;
		break;
	case 11025: // mp3 decoder SWIP = 11.025k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_11_025K;
		break;
	case 12000: // mp3 decoder SWIP = 12k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_12K;
		break;
	case 8000: // mp3 decoder SWIP = 8k
		hal_i2s_sample_rate = HAL_I2S_SAMPLE_RATE_8K;
		break;
	}
	return hal_i2s_sample_rate;
}

/* Example of i2s_configure */
static int8_t i2s_configure(MT768X_AUDIO_HANDLE *p)
{
	hal_i2s_config_t i2s_config;
	hal_i2s_status_t result = HAL_I2S_STATUS_OK;

	/* Set I2S as internal loopback mode */
	result = hal_i2s_init_ex(p->i2s_port, HAL_I2S_TYPE_EXTERNAL_MODE);
	if (HAL_I2S_STATUS_OK != result) {
		log_hal_info("aaaaa--------------aaaaa-------->>hal_i2s_init_ex failed --- %d\r\n", result);
		return -1;
	}

	/* Configure I2S  */
	i2s_config.clock_mode = HAL_I2S_MASTER;
	//i2s_config.rx_down_rate = HAL_I2S_RX_DOWN_RATE_DISABLE;
	i2s_config.tx_mode = HAL_I2S_TX_MONO_DUPLICATE_DISABLE;

    if(p->cfg.channelNum < 2)
    {
        i2s_config.i2s_out.channel_number = HAL_I2S_MONO;
        i2s_config.i2s_in.channel_number = HAL_I2S_MONO;
    }
    else
    {
        i2s_config.i2s_out.channel_number = HAL_I2S_STEREO;
        i2s_config.i2s_in.channel_number = HAL_I2S_STEREO;
    }

	i2s_config.i2s_out.sample_rate = sampleRate2HALIndex(p->cfg.sampleRate);
	i2s_config.i2s_in.sample_rate = sampleRate2HALIndex(p->cfg.sampleRate);
	i2s_config.i2s_in.msb_offset = 0;
	i2s_config.i2s_out.msb_offset = 0;
	i2s_config.i2s_in.word_select_inverse = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
	i2s_config.i2s_out.word_select_inverse = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
	i2s_config.i2s_in.lr_swap = HAL_I2S_LR_SWAP_DISABLE;
	i2s_config.i2s_out.lr_swap = HAL_I2S_LR_SWAP_DISABLE;

	i2s_config.sample_width = HAL_I2S_SAMPLE_WIDTH_16BIT;

	//log_hal_info("aaaaa--------------aaaaa-------->>hal_i2s_set_config_ex");
	result = hal_i2s_set_config_ex(p->i2s_port, &i2s_config);
	if (HAL_I2S_STATUS_OK != result) {
		log_hal_info("aaaaa--------------aaaaa-------->>hal_i2s_set_config_ex failed --- %d\r\n", result);
        hal_i2s_deinit_ex(p->i2s_port);
		return -1;
	}
    
	return 1;
}

static void i2s_tx_isr_callback(hal_i2s_event_t event, void *user_data)
{
	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)user_data;
	hal_i2s_disable_tx_dma_interrupt_ex(p->i2s_port);

	switch (event) {
	case HAL_I2S_EVENT_DATA_REQUEST:
		//log_hal_info("aaaaa--------------aaaaa-------->>output coming.");
		OAL_SemPost(&p->txWaitSem);
		break;
	}
}
static void i2s_rx_callback(hal_i2s_event_t event, void *user_data)
{
	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)user_data;
	hal_i2s_disable_rx_dma_interrupt_ex(p->i2s_port);

	switch (event) {
	case HAL_I2S_EVENT_DATA_REQUEST:
		break;
	case HAL_I2S_EVENT_DATA_NOTIFICATION:
		//log_hal_info("aaaaa--------------aaaaa-------->>input come");
		OAL_SemPost(&p->rxWaitSem);
		break;
	};
}

//////////////////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////////////////
void* OAL_OpenAudio(OAL_AUDIO_CFG *cfg)
{
    if(cfg->maxInputOutputSize > MAX_PCM_IO_LENGTH)
    {
        log_hal_info("aaaaa--------------aaaaa-------->>OAL_OpenAudio frame max length too large.");
        return 0;
    }

	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)OAL_calloc(1, sizeof(MT768X_AUDIO_HANDLE));
	if (!p)
		return 0;

	p->cfg = *cfg;
	p->txWaitSem = OAL_SemCreate(0);
	p->rxWaitSem = OAL_SemCreate(0);

    if (p->cfg.isInputMode)
    {
        //////////////////////////////////
        // !!! for fixed GPIO
        p->i2s_port = MT768X_I2S_OUTPUT_PORT;
        //////////////////////////////////
		log_hal_info("aaaaa--------------aaaaa-------->>OAL_OpenAudio input mode.\r\n");

#if MT768X_I2S_INPUT_PORT_HAS_I2C_CONFIG
        mt768x_audio_i2c_set_parameters(cfg->isInputMode, cfg->sampleRate, cfg->channelNum);
        mt768x_audio_i2c_set_input(MIC_IN);
#endif
		if (i2s_configure(p) < 0) { // Keep HAL_I2S_STEREO
			log_hal_info("aaaaa--------------aaaaa-------->>[mt768xaudio]: i2s_configure failed\r\n");
            goto _ERROR_RETURN;
		}
		hal_i2s_status_t result = hal_i2s_setup_rx_vfifo_ex(p->i2s_port, i2s_rx_buffer, cfg->maxInputOutputSize / 2, cfg->maxInputOutputSize);
		if (HAL_I2S_STATUS_OK != result) {
			log_hal_info("aaaaa--------------aaaaa-------->>[mt768xaudio]: hal_i2s_setup_rx_vfifo failed\r\n");
            goto _ERROR_RETURN;
		}
        

		result = hal_i2s_setup_tx_vfifo_ex(p->i2s_port, i2s_tx_buffer, cfg->maxInputOutputSize / 2, cfg->maxInputOutputSize);
		if (HAL_I2S_STATUS_OK != result) {
			log_hal_info("aaaaa--------------aaaaa-------->>[mt768xaudio]: hal_i2s_setup_tx_vfifo failed: %d\r\n", result);
            goto _ERROR_RETURN;
		}

		hal_i2s_register_rx_vfifo_callback_ex(p->i2s_port, i2s_rx_callback, p);
        // !!!!!!!!!!!!!!! we must enable tx for drive the rx !
        hal_i2s_enable_tx_ex(p->i2s_port);
		hal_i2s_enable_rx_ex(p->i2s_port);
		hal_i2s_enable_audio_top_ex(p->i2s_port);
        hal_i2s_enable_rx_dma_interrupt_ex(p->i2s_port);
	}
    else
    {
        //////////////////////////////////
        // !!! for fixed GPIO
        p->i2s_port = MT768X_I2S_INPUT_PORT;
        //////////////////////////////////
        log_hal_info("aaaaa--------------aaaaa-------->>OAL_OpenAudio output mode.\r\n");
#if MT768X_I2S_OUTPUT_PORT_HAS_I2C_CONFIG
        mt768x_audio_i2c_set_parameters(cfg->isInputMode, cfg->sampleRate, cfg->channelNum);
        mt768x_audio_i2c_set_output(SPK_OUT);
#endif
		if (i2s_configure(p) < 0) { // Keep HAL_I2S_STEREO
			log_hal_info("aaaaa--------------aaaaa-------->>[mt768xaudio]: i2s_configure failed\r\n");
            goto _ERROR_RETURN;
		}

		hal_i2s_status_t result = hal_i2s_setup_tx_vfifo_ex(p->i2s_port, i2s_tx_buffer, cfg->maxInputOutputSize / 2, cfg->maxInputOutputSize);
		if (HAL_I2S_STATUS_OK != result) {
			log_hal_info("aaaaa--------------aaaaa-------->>[mt768xaudio]: hal_i2s_setup_tx_vfifo failed: %d\r\n", result);
            goto _ERROR_RETURN;
		}

		hal_i2s_register_tx_vfifo_callback_ex(p->i2s_port, i2s_tx_isr_callback, p);
		hal_i2s_enable_tx_ex(p->i2s_port);
		hal_i2s_enable_audio_top_ex(p->i2s_port);
        hal_i2s_enable_tx_dma_interrupt_ex(p->i2s_port);
	}

	return (void *)p;
_ERROR_RETURN:
    OAL_SemDestroy(&p->txWaitSem);
    OAL_SemDestroy(&p->rxWaitSem);
    OAL_free(p);

    return 0;
}

void OAL_CloseAudio(void *handle)
{
	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)handle;

	if (p->cfg.isInputMode) {
        //hal_i2s_disable_tx_dma_interrupt_ex(p->i2s_port);
        hal_i2s_disable_rx_dma_interrupt_ex(p->i2s_port);
        hal_i2s_disable_tx_ex(p->i2s_port);
		hal_i2s_disable_rx_ex(p->i2s_port);
    }else{
		hal_i2s_disable_tx_dma_interrupt_ex(p->i2s_port);
		hal_i2s_disable_tx_ex(p->i2s_port);
	}
	hal_i2s_disable_audio_top_ex(p->i2s_port);
	hal_i2s_deinit_ex(p->i2s_port);

    if (p->cfg.isInputMode) {
        hal_i2s_stop_tx_vfifo_ex(p->i2s_port);
		hal_i2s_stop_rx_vfifo_ex(p->i2s_port);
	}
	else {
		hal_i2s_stop_tx_vfifo_ex(p->i2s_port);
	}

	OAL_SemDestroy(&p->txWaitSem);
	OAL_SemDestroy(&p->rxWaitSem);
	OAL_free(p);
}

static void outputPCM(MT768X_AUDIO_HANDLE *p, unsigned int* data, unsigned int length)
{
	for (int i = 0; i < length / 4; i++) {
		hal_i2s_tx_write_ex(p->i2s_port, *data);
		data++;
	}
}

void OAL_OutputPCM(void *handle, unsigned int* data, unsigned int length)
{
	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)handle;
	if (p->remainNeedLength > 0) {
		unsigned int realpush = MINIMUM(p->remainNeedLength, length);
		outputPCM(p, data, realpush);

		p->remainNeedLength -= realpush;
		length -= realpush;
		data += (realpush / 4);
	}
	while (length > 0) {
		OAL_SemWait(&p->txWaitSem);
		unsigned int i2s_amount_space_tones;
		hal_i2s_get_tx_sample_count_ex(p->i2s_port, &i2s_amount_space_tones);
		p->remainNeedLength = i2s_amount_space_tones * 2;

		unsigned int realpush = MINIMUM(i2s_amount_space_tones * 2, length);
		outputPCM(p, data, realpush);

		p->remainNeedLength -= realpush;
		length -= realpush;
		data += (realpush / 4);
	}

	hal_i2s_enable_tx_dma_interrupt_ex(p->i2s_port);
}

static void inputPCM(MT768X_AUDIO_HANDLE *p, unsigned int* data, unsigned int length)
{
	for (int i = 0; i < length / 4; i++) {
		hal_i2s_rx_read_ex(p->i2s_port, data);
		data++;
	}
}

void OAL_InputPCM(void *handle, unsigned int* data, unsigned int length)
{
	MT768X_AUDIO_HANDLE *p = (MT768X_AUDIO_HANDLE *)handle;
    int readBytes = 0;
	if (p->remainNeedLength > 0) {
		unsigned int realpush = MINIMUM(p->remainNeedLength, length);
		inputPCM(p, data, realpush);

        readBytes = (realpush - realpush%4);
		p->remainNeedLength -= readBytes;
		length -= readBytes;
		data += (readBytes / 4);
	}
	while (length > 0) {
		OAL_SemWait(&p->rxWaitSem);

		unsigned int i2s_amount_incoming_tones;
		hal_i2s_get_rx_sample_count_ex(p->i2s_port, &i2s_amount_incoming_tones);
		p->remainNeedLength = i2s_amount_incoming_tones * 2;
		unsigned int realpush = MINIMUM(i2s_amount_incoming_tones * 2, length);
		inputPCM(p, data, realpush);

        readBytes = (realpush - realpush%4);
		p->remainNeedLength -= readBytes;
		length -= readBytes;
		data += (readBytes / 4);
	}

	hal_i2s_enable_rx_dma_interrupt_ex(p->i2s_port);
}


int OAL_SetVolume(int volume)
{
    mt768x_audio_i2c_set_volume((unsigned char)volume);
	return 0;
}

int OAL_GetVolume()
{
	return (int)mt768x_audio_i2c_get_volume();
}

int OAL_SetMute(int onOff)
{
	return 0;
}

int OAL_IsHeadphoneExist()
{
	return 0;
}

void OAL_InitAudio() // only call once when power on
{
	log_hal_info("aaaaa--------------aaaaa-------->>OAL_InitAudio.\r\n");
    printf("60477 before OAL_InitAudiogpio mode= %d,\n", GPIO_ReadReg(0xA20B0044));

    mt768x_audio_i2c_init();
	
    printf("60477 after OAL_InitAudiogpio mode= %d,\n", GPIO_ReadReg(0xA20B0044));
}
void OAL_UninitAudio()
{
}
