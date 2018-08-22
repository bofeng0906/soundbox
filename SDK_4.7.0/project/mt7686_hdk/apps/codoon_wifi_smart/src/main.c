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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "sys_init.h"
#include "wifi_nvdm_config.h"
#include "wifi_lwip_helper.h"
//////////////////////////////////////////////////////////////////////////
#include "smt_api.h"
#include "task_def.h"
#include "smt_conn.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////
#if defined(MTK_MINICLI_ENABLE)
#include "cli_def.h"
#endif

#include "bsp_gpio_ept_config.h"
#include "hal_sleep_manager.h"

#include "connsys_profile.h"
#include "wifi_api.h"

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#include "hal_wdt.h"
#endif

/* application includes */
#if defined(MTK_AUDIO_MP3_ENABLED)
#include "mp3_codec_demo.h"
#include "task_def.h"
#endif

#ifdef MTK_AUDIO_AMR_ENABLED
#include "amr_decoder.h"
#include "amr_decoder_demo.h"
#include "amr_encoder.h"
#include "amr_encoder_demo.h"
#include "task_def.h"
#endif

#ifdef MTK_AUDIO_AAC_DECODER_ENABLED
#include "task_def.h"
#include "aac_play_demo.h"
#endif

#if defined(MTK_AUDIO_MP3_ENABLED) || defined(MTK_AUDIO_AMR_ENABLED)
#include "audip_exp.h"
IPCOMMON_PLUS
#endif

#ifdef MTK_ATCI_ENABLE
#include "atci.h"
#include "at_command_wifi.h"

/**
 * @brief This function is a task main function for processing the data handled by ATCI module.
 * @param[in] param is the task main function paramter.
 * @return    None
 */
static void atci_def_task(void *param)
{

    LOG_I(common, "enter atci_def_task!!\n\r");
    while (1) {
        atci_processing();
    }
}

#endif

int32_t wifi_station_port_secure_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
int32_t wifi_scan_complete_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE

#ifdef HAL_WDT_MODULE_ENABLED
void wdt_timeout_handle(hal_wdt_reset_status_t wdt_reset_status)
{
    printf("%s: stattus:%u\r\n", __FUNCTION__, (unsigned int)wdt_reset_status);
    /* assert 0 to trigger exception hanling flow */
    configASSERT(0);
}

static void wdt_init(void)
{
    hal_wdt_config_t wdt_init;
    wdt_init.mode = HAL_WDT_MODE_INTERRUPT;
    wdt_init.seconds = 15;
    hal_wdt_init(&wdt_init);
    hal_wdt_register_callback(wdt_timeout_handle);
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
}
#endif
#endif


/* for idle task feed wdt (DO NOT enter sleep mode)*/
void vApplicationIdleHook(void)
{
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
    hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif
#endif
}
///////////////////////////////////////////////////////////////////////////////////////
int32_t wifi_init_done_handler(wifi_event_t event,
                                      uint8_t *payload,
                                      uint32_t length)
{
    LOG_I(common, "WiFi Init Done: port = %d", payload[6]);
    return 1;
}


#define WIFI_SSID                ("codoon_wifi")
#define WIFI_PASSWORD            ("WNKZ8519hk")
/* for getlink task */
#define GETLINK_TASK_NAME              "getlink"
#define GETLINK_TASK_STACKSIZE         (512*4) /*unit byte!*/
#define GETLINK_TASK_PRIO              TASK_PRIORITY_NORMAL

/* for codoonaudio task */
#define CODOONAUDIO_TASK_NAME           "codoonaudio"
#define CODOONAUDIO_TASK_STACKSIZE      (1024*4) /*unit byte!*/
#define CODOONAUDIO_TASK_PRIO           TASK_PRIORITY_NORMAL

/* User-defined wifi event callback function */
bool g_wifi_init_ready = false;
int32_t user_wifi_event_callback(wifi_event_t event, uint8_t *payload, uint32_t length)
{
	switch (event)
	{
	    case WIFI_EVENT_IOT_INIT_COMPLETE:
	    	 LOG_I(common, "WiFi Init Done: port = %d", payload[6]);
	    	 g_wifi_init_ready = true;
	    	 break;
	    case WIFI_EVENT_IOT_CONNECTED:
			 if ((length == WIFI_MAC_ADDRESS_LENGTH) && (payload)) {
			 printf("[User Event Callback Sample]: LinkUp! CONNECTED MAC\
				 = %02x:%02x:%02x:%02x:%02x:%02x\r\n",\
				 payload[0],payload[1],payload[2],\
				 payload[3],payload[4],payload[5]);
			 } else {
			     printf("[User Event Callback Sample]: LinkUp!\r\n");
			     printf("[User Event Callback Sample]: LinkUp! CONNECTED MAC\
			     				 = %02x:%02x:%02x:%02x:%02x:%02x\r\n",\
			     				 payload[0],payload[1],payload[2],\
			     				 payload[3],payload[4],payload[5]);
			 }
			 break;
	    case WIFI_EVENT_IOT_SCAN_COMPLETE:
			 printf("[User Event Callback Sample]: Scan Done!\r\n");
			 break;
	    case WIFI_EVENT_IOT_DISCONNECTED:
	    if ((length == WIFI_MAC_ADDRESS_LENGTH) && (payload)) {
			 printf("[User Event Callback Sample]: Disconnect! DISCONNECTED MAC\
				 = %02x:%02x:%02x:%02x:%02x:%02x\n",\
				 payload[0],payload[1],payload[2],\
				 payload[3],payload[4],payload[5]);
			} else {
			     printf("[User Event Callback Sample]: Disconnect!\r\n");
			     printf("[User Event Callback Sample]: Disconnect! DISCONNECTED MAC\
			    				 = %02x:%02x:%02x:%02x:%02x:%02x\r\n",\
			    				 payload[0],payload[1],payload[2],\
			    				 payload[3],payload[4],payload[5]);
			}
			break;
	    case WIFI_EVENT_IOT_PORT_SECURE:
	    if ((length == WIFI_MAC_ADDRESS_LENGTH) && (payload)) {
			 printf("[User Event Callback Sample]: Port Secure! CONNECTED MAC\
				 = %02x:%02x:%02x:%02x:%02x:%02x\n",\
				 payload[0],payload[1],payload[2],\
				 payload[3],payload[4],payload[5]);
			 } else {
			     printf("[User Event Callback Sample]: Port Secure! \r\n");
			 }
			break;
	    default:
	         printf("[User Event Callback Sample]: Unknown event(%d)\r\n",event);
	    break;
	}
	return -1;
}
xQueueHandle MsgQueue; //声明一个队列句柄
/* User-defined task */
void getlink_task(void *pvParameters)
{
	uint8_t status = 0;
	uint8_t link = 0;
	int16_t SendNum = 0;
	while(1)
	{
		printf("getlink_task running!\r\n");
		status = wifi_connection_get_link_status(&link);
		if (link == 1){
		   printf("link=%d, the station is connecting to an AP router!\r\n", link);
		   printf("smart_connection_ssid:%s\r\n", WIFI_SSID);
		   printf("smart_connection_password:%s\r\n", WIFI_PASSWORD);
		   SendNum = 1;
		   xQueueSend( MsgQueue, ( void* )&SendNum, 0 );//传输队列项
		}else if (link == 0) {
		   printf("link=%d, the station not connect to an AP router!\r\n",link);
		   SendNum = 1;
		   xQueueSend( MsgQueue, ( void* )&SendNum, 0 );//传输队列项
		}
		vTaskDelay(10000);
	}
}
static FATFS fatfs;
static uint8_t first_time_f_mount = 1;
/* User-defined task */
void codoonaudio_task(void *pvParameters)
{
	int16_t receivenum;
	while(1)
	{
    //if(uxQueueMessagesWaiting(MsgQueue)!=0)
    //{
        if(xQueueReceive( MsgQueue, (void*)&receivenum ,  10 )== pdPASS )
        {
        	if(receivenum==1)
        	{
				printf("/******************************************************************/\r\n");
				printf("mp3_codec_demo_task running!\r\n");
				uint8_t res=0;
				if (first_time_f_mount){
					printf("f_mount() +\r\n");
					res = f_mount(&fatfs, _T("0:"), 1);
					printf("f_mount() -\r\n");
					first_time_f_mount = 0;
				} else {
					printf("already fmount\r\n");
					res = FR_OK;
				}
				if (!res) {
					printf("fmount ok \r\n");
				} else {
					printf("fmount error \r\n");
				}
				res=SynMp3files ("SD:/temp",15);
				//if(res==1)mp3_codec_demo_play_stop();
				//if(res==1)mp3_codec_demo_play_pause_resume();
				if(res==1)codoon_audio_play();
				printf("/*************************end****************************/\r\n");
			    vTaskDelay(10000 / portTICK_RATE_MS);
				while (0)
				{
					vTaskDelay(100 / portTICK_RATE_MS);
				}
        	}
            printf("ReceiveNum:%d\r\n",receivenum);
        }
    //}
    vTaskDelay(10000 / portTICK_RATE_MS);
   }
}
///////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	int32_t res;
    /* Do system initialization, eg: hardware, nvdm, logging and random seed. */
    system_init();
    printf("/******************************************************************/\r\n");
    printf("MT7686DN + SDK_V4.7.0 : CODOON_WIFI_SMART !\r\n");
    printf("/******************************************************************/\r\n");
#if 0 // Disable WiFi because assert in booting
    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
         * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
	wifi_cfg_t wifi_config = {0};
	if (0 != wifi_config_init(&wifi_config)) {
		LOG_E(common, "wifi config init fail");
		return -1;
	}
	wifi_config_t config = {0};
	wifi_config_ext_t config_ext = {0};

	config.opmode = wifi_config.opmode;

	memcpy(config.sta_config.ssid, wifi_config.sta_ssid, 32);
	config.sta_config.ssid_length = wifi_config.sta_ssid_len;
	config.sta_config.bssid_present = 0;
	//os_memcpy(config.sta_config.bssid, target_bssid, WIFI_MAC_ADDRESS_LENGTH);
	memcpy(config.sta_config.password, wifi_config.sta_wpa_psk, 64);
	config.sta_config.password_length = wifi_config.sta_wpa_psk_len;
	config_ext.sta_wep_key_index_present = 1;
	config_ext.sta_wep_key_index = wifi_config.sta_default_key_id;
	config_ext.sta_auto_connect_present = 1;
	config_ext.sta_auto_connect = 1;
	/*
	memcpy(config.ap_config.ssid, wifi_config.ap_ssid, 32);
	config.ap_config.ssid_length = wifi_config.ap_ssid_len;
	memcpy(config.ap_config.password, wifi_config.ap_wpa_psk, 64);
	config.ap_config.password_length = wifi_config.ap_wpa_psk_len;
	config.ap_config.auth_mode = (wifi_auth_mode_t)wifi_config.ap_auth_mode;
	config.ap_config.encrypt_type = (wifi_encrypt_type_t)wifi_config.ap_encryp_type;
	config.ap_config.channel = wifi_config.ap_channel;
	config.ap_config.bandwidth = wifi_config.ap_bw;
	config.ap_config.bandwidth_ext = WIFI_BANDWIDTH_EXT_40MHZ_UP;
	config_ext.ap_wep_key_index_present = 1;
	config_ext.ap_wep_key_index = wifi_config.ap_default_key_id;
	config_ext.ap_hidden_ssid_enable_present = 1;
	config_ext.ap_hidden_ssid_enable = wifi_config.ap_hide_ssid;
	config_ext.sta_power_save_mode = wifi_config.sta_power_save_mode;
	*/
	printf("config.sta_config.ssid:%s\r\n", config.sta_config.ssid);
	printf("config.sta_config.ssid_length:%d\r\n", config.sta_config.ssid_length);
	printf("config.sta_config.password:%s\r\n", config.sta_config.password);
	printf("config.sta_config.password_length:%d\r\n", config.sta_config.password_length);
	printf("\r\n");

	///*
	config.opmode = WIFI_MODE_STA_ONLY;
	strcpy((char *)config.sta_config.ssid, WIFI_SSID);
	strcpy((char *)config.sta_config.password, WIFI_PASSWORD);
	config.sta_config.ssid_length = strlen(WIFI_SSID);
	config.sta_config.password_length = strlen(WIFI_PASSWORD);

	printf("config.opmode:%d\r\n", config.opmode);
	printf("config.sta_config.ssid:%s\r\n", config.sta_config.ssid);
	printf("config.sta_config.ssid_length:%d\r\n", config.sta_config.ssid_length);
	printf("config.sta_config.password:%s\r\n", config.sta_config.password);
	printf("config.sta_config.password_length:%d\r\n", config.sta_config.password_length);
	printf("\r\n");
	//*/

	/* Initialize wifi stack and register wifi init complete event handler,
	 * notes:  the wifi initial process will be implemented and finished while system task scheduler is running,
	 *            when it is done , the WIFI_EVENT_IOT_INIT_COMPLETE event will be triggered */
	wifi_init(&config, &config_ext);

	/* Register wifi initialization event handler in main() function */
	res=wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE, user_wifi_event_callback);
	if(res>=0){printf("wifi_eventinitcomplete_register!\r\n");}

	/* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
	lwip_network_init(config.opmode);
	lwip_net_start(config.opmode);

#endif

#if defined(MTK_MINICLI_ENABLE)
    printf("/******************************************************************/\r\n");
    /* Initialize cli task to enable user input cli command from uart port.*/
    cli_def_create();
    cli_task_create();
#endif

#ifdef MTK_ATCI_ENABLE
    printf("atci ok!\r\n");
    /* init ATCI module and set UART port */
    atci_init(HAL_UART_2);
    /* create task for ATCI */
    xTaskCreate(atci_def_task, ATCI_TASK_NAME, ATCI_TASK_STACKSIZE / ((uint32_t)sizeof(StackType_t)), NULL, ATCI_TASK_PRIO, NULL);
#if 1 // Disable WiFi because assert in booting
#if defined(MTK_WIFI_AT_COMMAND_ENABLE)
    wifi_atci_example_init();
#endif
#endif
#endif



#if defined(MTK_AUDIO_MP3_ENABLED)

    /*
    hal_gpio_init(HAL_GPIO_0);
    hal_gpio_init(HAL_GPIO_1);
    hal_gpio_pull_up(HAL_GPIO_0);
    hal_gpio_pull_up(HAL_GPIO_1);
    hal_pinmux_set_function(HAL_GPIO_0, 4);
    hal_pinmux_set_function(HAL_GPIO_1, 4);
    */

    printf("/******************************************************************/\r\n");
    //create task for codoonaudio
    printf("user_codoonaudio_task create!\r\n");
    xTaskCreate(codoonaudio_task, CODOONAUDIO_TASK_NAME, CODOONAUDIO_TASK_STACKSIZE / sizeof(StackType_t), NULL, CODOONAUDIO_TASK_PRIO, NULL);
#endif

    printf("/******************************************************************/\r\n");
    /* create task for getlink */
    printf("user_getlink_task create!\r\n");
    xTaskCreate(getlink_task, GETLINK_TASK_NAME, GETLINK_TASK_STACKSIZE / ((uint32_t)sizeof(StackType_t)), NULL, GETLINK_TASK_PRIO, NULL);
    printf("/******************************************************************/\r\n");

    /* Call this function to indicate the system initialize done. */
    SysInitStatus_Set();
    printf("/******************************************************************/\r\n");
 	MsgQueue = xQueueCreate( 5 , sizeof( int16_t) );//创建队列
 	if(MsgQueue!=0){
 		printf("create msgqueue ok!\r\n");
 	}
 	printf("/******************************************************************/\r\n");

    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

