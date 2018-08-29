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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "sys_init.h"
#include "task_def.h"

#include "syslog.h"
#include "oal_audio.h"
#include "pcm44100.h"

#include "hal_gpio.h"

/****************************************************************************
 *
 * CLI Example Code
 *
 ****************************************************************************/


/* Include this file for CLI engine prototypes definition. */
#include "cli.h"

/* Include this file for I/O interface API default implementation */
#include "io_def.h"

/* Include this file for integer parsing */
#include "toi.h"


/*	getchar, putchar declaration  */
//GETCHAR_PROTOTYPE;
//PUTCHAR_PROTOTYPE;


#define HISTORY_LINE_MAX    (128)
#define HISTORY_LINES       (20)

static char s_history_lines[ HISTORY_LINES ][ HISTORY_LINE_MAX ];
static char *s_history_ptrs[ HISTORY_LINES ];

static char s_history_input[ HISTORY_LINE_MAX ];
static char s_history_parse_token[ HISTORY_LINE_MAX ];


/* Example of user custom CLI command, note that the prototype matches cli_cmd_handler_t */
static uint8_t _example_cli_command_hello(uint8_t argc, char *argv[])
{
    printf("Hello world\n");
    return 0;
}


static uint8_t _example_cli_command_echo(uint8_t argc, char *argv[])
{
    int i;

    for (i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }

    printf("\n");

    return 0;
}
void cmd_line_playtone(int argc, char *argv[])
{
    int i;
    int playtime;
    //int sz = (acodec_cfg.mclk_freq/512)/freq; /* single wave data size */
    int sz = 48;
    unsigned char buf[sz * 2]; /* since 16 bit data */

    /* create a data for triangular wave to output */
    for (i = 0; i < sz; i++) {
        buf[i] = i * 2;
        buf[i + sz] = (sz - i) * 2;
    }

    /* Output same wave for needed time duration */
    // argv[1] as second
    playtime = 3000;
    OAL_AUDIO_CFG cfg = {0,0, sz*2,16000,1,16};
    void *p = OAL_OpenAudio(&cfg);
    if(p){
        while (playtime--)
            OAL_OutputPCM(p, buf, sz *2);

        OAL_CloseAudio(p);
    }
}

void cmd_line_playsong(int argc, char *argv[])
{
    OAL_AUDIO_CFG cfg = {0,0, 2048, 44100,2,16};
    void *p = OAL_OpenAudio(&cfg);

    int max_step = sizeof(II_PCM)/2048;
    for(int i=0; i< max_step; i ++)
    {
        OAL_OutputPCM(p, II_PCM +i*2048, 2048);
    }
    OAL_CloseAudio(p);
}
static record_buf[1024];
static void _record_task(void *param)
{
	OAL_AUDIO_CFG cfg = {1, 0, 1024, 16000, 2, 16};

	void *p = OAL_OpenAudio(&cfg);
	
	printf("i2s0 is opened for record.\r\n");

#if 1
	int i;
	for(i =0; i < 10000; i ++)
	{
		OAL_InputPCM(p, record_buf, 1024);
		//vTaskDelay(100);
		printf(".");
		if(i % 100 == 0)
			printf("\r\n");
	}
	
	OAL_CloseAudio(p);
#else
	while(1)
		vTaskDelay(1000);
#endif	
	vTaskDelete(NULL);
}
void cmd_line_recordtone(int argc, char *argv[])
{
    xTaskCreate(_record_task,
                "reco",
                APP_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                NULL,
                APP_TASK_PRIO,
                NULL);
}
static uint8_t _example_cli_command_int(uint8_t argc, char *argv[])
{
    int         i;
    uint8_t     type;
    uint32_t    value;

    for (i = 0; i < argc; i++) {

        value = toi(argv[i], &type);
        switch (type) {
            case TOI_ERR:
                printf("'%s' is not a valid integer\n", argv[i]);
                break;
            case TOI_BIN:
                printf("'%s' is a binary number, value is %u in decimal\n", argv[i], (unsigned int)value);
                break;
            case TOI_OCT:
                printf("'%s' is an octal number, value is %u in decimal\n", argv[i], (unsigned int)value);
                break;
            case TOI_DEC:
                printf("'%s' is a decimal number, value is %u\n", argv[i], (unsigned int)value);
                break;
            case TOI_HEX:
                printf("'%s' is a hexadecimal number, value is %u in decimal\n", argv[i], (unsigned int)value);
            default:
                printf("this never happens!\n");
                break;
        }
    }

    return 0;
}


/* CLI Command list
   Format:
   {<Command name>, <Command help message>, <Command function>, <Sub command (cmd_t)>}\

   NOTE:
   The last one must be {NULL, NULL, NULL, NULL}
*/
static cmd_t  _cmds_normal[] = {
    { "hello", "hello world",               _example_cli_command_hello, NULL },
    { "echo",  "echo input 'parameters'",   _example_cli_command_echo,  NULL },
    { "parse", "parse input 'parameters'",  _example_cli_command_int,   NULL },
    { "play16000", "", cmd_line_playtone, NULL },
	{ "play44100", "", cmd_line_playsong, NULL },
	{ "record", "", cmd_line_recordtone, NULL },
    /*	Add your custom command here */
    { NULL, NULL, NULL, NULL }
};


/* CLI control block */
static cli_t _cli_cb = {
    .state  = 1,
    .echo   = 0,
    .get    = __io_getchar,
    .put    = __io_putchar,
    .cmd	= &_cmds_normal[0]
};


/* FreeRTOS task of CLI */
static void _example_cli_task(void *param)
{
    int i;

    /**
     *  Prepare CLI history buffer
     */
    cli_history_t *hist = &_cli_cb.history;

    for (i = 0; i < HISTORY_LINES; i++) {
        s_history_ptrs[i] = s_history_lines[i];
    }

    hist->history           = &s_history_ptrs[0];
    hist->input             = s_history_input;
    hist->parse_token       = s_history_parse_token;
    hist->history_max       = HISTORY_LINES;
    hist->line_max          = HISTORY_LINE_MAX;
    hist->index             = 0;
    hist->position          = 0;
    hist->full              = 0;

    /**
     *  Init CLI control block
     */
    cli_init(&_cli_cb);

    for (;;) {
        cli_task();
    }
}


/****************************************************************************
 *
 * C Langauge Entry Point
 *
 ****************************************************************************/


/* Create the log control block as user wishes. Here we use 'template' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(minicli_proj, PRINT_LEVEL_INFO);


int main(void)
{
    /*
     * Do system initialization, eg: hardware, nvdm and random seed.
     */

    system_init();

    /*
     * system log initialization.
     * This is the simplest way to initialize system log, that just inputs three NULLs
     * as input arguments. User can use advanved feature of system log along with NVDM.
     * For more details, please refer to the log dev guide under /doc folder or projects
     * under project/mtxxxx_hdk/apps/.
     */

    //hal_pinmux_set_function(HAL_GPIO_15, 5);
    //hal_pinmux_set_function(HAL_GPIO_16, 5);

    /*Step1: Initialize GPIO, set GPIO pinmux(if EPT tool is not used to configure the related pinmux) */    
    hal_gpio_init(HAL_GPIO_15);    
    hal_gpio_init(HAL_GPIO_16);    
    /* Call hal_pinmux_set_function() to set GPIO pinmux.*/    
    hal_pinmux_set_function(HAL_GPIO_15, HAL_GPIO_15_SCL1);    
    hal_pinmux_set_function(HAL_GPIO_16, HAL_GPIO_16_SDA1);

    hal_gpio_init(HAL_GPIO_5); 
    hal_gpio_init(HAL_GPIO_6);    
    hal_gpio_init(HAL_GPIO_7);    
    hal_gpio_init(HAL_GPIO_8);    
    hal_gpio_init(HAL_GPIO_9);  

    hal_pinmux_set_function(HAL_GPIO_5, HAL_GPIO_5_TDM_RX);    
    hal_pinmux_set_function(HAL_GPIO_6, HAL_GPIO_6_TDM_TX);    
    hal_pinmux_set_function(HAL_GPIO_7, HAL_GPIO_7_TDM_WS);
    hal_pinmux_set_function(HAL_GPIO_8, HAL_GPIO_8_TDM_CK);    
    hal_pinmux_set_function(HAL_GPIO_9, HAL_GPIO_9_TDM_MCLK);

    hal_gpio_init(HAL_GPIO_17); 
    hal_gpio_set_direction(HAL_GPIO_17, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(HAL_GPIO_17, HAL_GPIO_DATA_HIGH);

    
    log_init(NULL, NULL, NULL);

	OAL_InitAudio();
	
    LOG_I(minicli_proj, "start to create task");

    /*
     * Create a user task for demo when and how to use wifi config API to change WiFI settings,
     * Most WiFi APIs must be called in task scheduler, the system will work wrong if called in main(),
     * For which API must be called in task, please refer to wifi_api.h or WiFi API reference.
     * xTaskCreate(user_wifi_app_entry,
     *             UNIFY_USR_DEMO_TASK_NAME,
     *             UNIFY_USR_DEMO_TASK_STACKSIZE / 4,
     *             NULL, UNIFY_USR_DEMO_TASK_PRIO, NULL);
     * user_wifi_app_entry is user's task entry function, which may be defined in another C file to do application job.
     * UNIFY_USR_DEMO_TASK_NAME, UNIFY_USR_DEMO_TASK_STACKSIZE and UNIFY_USR_DEMO_TASK_PRIO should be defined
     * in task_def.h. User needs to refer to example in task_def.h, then makes own task MACROs defined.
     */

    xTaskCreate(_example_cli_task,
                APP_TASK_NAME,
                APP_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                NULL,
                APP_TASK_PRIO,
                NULL);

    /*
     * Call this function to indicate the system initialize done.
     */

    SysInitStatus_Set();

    /*
     * Start the scheduler.
     */

    vTaskStartScheduler();

    /*
     * If all is well, the scheduler will now be running, and the following line
     * will never be reached.  If the following line does execute, then there was
     * insufficient FreeRTOS heap memory available for the idle and/or timer tasks
     * to be created.  See the memory management section on the FreeRTOS web site
     * for more details.
     */

    for( ;; );
}
