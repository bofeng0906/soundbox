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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
//#include "debug_interaction.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mp3_codec_demo.h"
#include "mp3_codec.h"
#include "hal_audio.h"
#include "hal_log.h"
#include "ff.h"
#include "hal_gpt.h"
#include "queue.h"

#define MP3_CODEC_UT_PLAY_PAUSE_RESUME_10TIMES (0) // for QA test only SDK v4.5.0
#define MP3_CODEC_DEMO_PLAY_STOP_STEREOMONO_SR_8K_TO_48K (0)

static FATFS fatfs;
static FIL fdst;
static FRESULT res;
static UINT length_read;
static uint8_t first_time_f_mount = 1;

static int16_t test_open_file_from_sd(FIL *fp, const TCHAR *path)
{
    FRESULT res;
    printf("test_open_file_from_sd() +\r\n");
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
        res = f_open(fp, path, FA_OPEN_EXISTING | FA_WRITE | FA_READ);
        if (!res) {
            printf("fopen ok \r\n");
        } else {
            printf("fopen error \r\n");
            return -1;
        }
    } else {
        printf("fmount error \r\n");
    }
    printf("test_open_file_from_sd() -\r\n");
    if (res != FR_OK) {
        return -1;
    } else {
        return 0;
    }
}

static void test_stop_read_from_sd(mp3_codec_media_handle_t *hdl)
{
    hdl->stop(hdl);
    res = f_close(&fdst);
    if (!res) {
        printf("[MP3 Codec Demo] fclose success \r\n");
    } else {
        printf("[MP3 Codec Demo] fclose error \r\n");
    }
    hdl->close_codec(hdl);
    printf("[MP3 Codec Demo] close codec() - \r\n");
}

static void test_sd_event_callback(mp3_codec_media_handle_t *hdl, mp3_codec_event_t event)
{
    uint8_t *share_buf;
    uint32_t share_buf_len;
    switch (event) {
        case MP3_CODEC_MEDIA_JUMP_FILE_TO:
            res = f_lseek(&fdst, (DWORD)hdl->jump_file_to_specified_position);
            // printf("[MP3 Codec Demo] MP3_CODEC_MEDIA_JUMP_FILE_TO: fdst read/write pointer=%x\r\n", f_tell(&fdst));
            break;
        case MP3_CODEC_MEDIA_BITSTREAM_END:
            printf("[MP3 Codec ut] [MP3_CODEC_MEDIA_BITSTREAM_END] Stop mp3\n");
            test_stop_read_from_sd(hdl);
            break;
        case MP3_CODEC_MEDIA_REQUEST:
        case MP3_CODEC_MEDIA_UNDERFLOW:
            if (f_eof(&fdst)) {
                printf("[MP3 Codec ut] [EOF] End of this mp3 file. Flush mp3\n");
                hdl->flush(hdl, 1);
                break;
            }
            uint32_t loop_idx;
            uint32_t loop_cnt = 2;
            for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
                hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
                f_read(&fdst, share_buf, share_buf_len, &length_read);
                hdl->write_data_done(hdl, share_buf_len);
            }
            hdl->finish_write_data(hdl);
            break;
    }
}

void mp3_codec_demo_play_stop()
{
    mp3_codec_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    // Open the mp3 codec.
    printf("/******************************************************************/\r\n");
    printf("[MP3 Codec] open mp3 codec\r\n");
    hdl = mp3_codec_open(test_sd_event_callback);
    if (hdl == NULL) {
        printf("[MP3 Codec] Fail to open the codec.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    printf("[MP3 Codec] mp3_codec_set_memory2() +\r\n");
    mp3_codec_set_memory2();
    printf("[MP3 Codec] mp3_codec_set_memory2() -\r\n");
    if (test_open_file_from_sd(&fdst, _T("SD:/temp/synfile.mp3")) < 0) {
        printf("[MP3 Codec] Fail to open SD:/temp/synfile.mp3.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
    //printf("/******************************************************************/\r\n");
    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    hdl->write_data_done(hdl, share_buf_len);
    hdl->finish_write_data(hdl);
    hdl->skip_id3v2_and_reach_next_frame(hdl, (uint32_t)f_size(&fdst));

    printf("[MP3 Codec Demo] play +\r\n");
    hdl->play(hdl);
    printf("[MP3 Codec Demo] play -\r\n");
    //printf("/******************************************************************/\r\n");
}

void mp3_codec_demo_play_pause_resume()
{
    mp3_codec_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    // Open the mp3 codec.
    printf("[MP3 Codec Demo] open mp3 codec\r\n");
    hdl = mp3_codec_open(test_sd_event_callback);
    if (hdl == NULL) {
        log_hal_error("[MP3 Codec Demo] Fail to open the codec.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    mp3_codec_set_memory2();

    if (test_open_file_from_sd(&fdst, _T("SD:/temp/synfile.mp3")) < 0) {
        log_hal_error("[MP3 Codec Demo] Fail to open SD:/temp/synfile.mp3.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    hdl->write_data_done(hdl, share_buf_len);
    hdl->finish_write_data(hdl);
    hdl->skip_id3v2_and_reach_next_frame(hdl, (uint32_t)f_size(&fdst));

    printf("[MP3 Codec Demo] play +\r\n");
    hdl->play(hdl);
    printf("[MP3 Codec Demo] play -\r\n");

    printf("[MP3 Codec Demo] vTaskDelay(5000 / portTICK_RATE_MS); +\r\n");
    vTaskDelay(5000 / portTICK_RATE_MS);
    printf("[MP3 Codec Demo] vTaskDelay(5000 / portTICK_RATE_MS); -\r\n");

    printf("[MP3 Codec Demo] pause +\r\n");
    hdl->pause(hdl);
    printf("[MP3 Codec Demo] pause -\r\n");    

    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); +\r\n");
    hal_gpt_delay_ms(3000);
    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); -\r\n");


    printf("[MP3 Codec Demo] resume() +\r\n");
    hdl->resume(hdl);
    printf("[MP3 Codec Demo] resume() -\r\n");

}

/////////////////////////////////////////////////////////////////////////////////////
void codoon_audio_play()
{
    mp3_codec_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    // Open the mp3 codec.
    printf("[MP3 Codec Demo] open mp3 codec\r\n");
    hdl = mp3_codec_open(test_sd_event_callback);
    if (hdl == NULL) {
        log_hal_error("[MP3 Codec Demo] Fail to open the codec.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    mp3_codec_set_memory2();

    if (test_open_file_from_sd(&fdst, _T("SD:/temp/synfile.mp3")) < 0) {
        log_hal_error("[MP3 Codec Demo] Fail to open SD:/temp/synfile.mp3.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    hdl->write_data_done(hdl, share_buf_len);
    hdl->finish_write_data(hdl);
    hdl->skip_id3v2_and_reach_next_frame(hdl, (uint32_t)f_size(&fdst));

    printf("[MP3 Codec Demo] play +\r\n");
    hdl->play(hdl);
    printf("[MP3 Codec Demo] play -\r\n");
    /////////////////////////////////////////////////////////////////////
    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); +\r\n");
       hal_gpt_delay_ms(3000);
       printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); -\r\n");


    printf("[MP3 Codec Demo] stop +\r\n");
    hdl->stop(hdl);
    printf("[MP3 Codec Demo] stop -\r\n");

    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); +\r\n");
    hal_gpt_delay_ms(3000);
    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); -\r\n");


    printf("[MP3 Codec Demo] close_codec() +\r\n");
    hdl->close_codec(hdl);
    printf("[MP3 Codec Demo] close_codec() -\r\n");

}
/////////////////////////////////////////////////////////////////////////////////////
extern xQueueHandle MsgQueue; //声明一个队列句柄
void mp3_codec_demo(void *pvParameters)
{
	int16_t receivenum;
	while(1)
	{
    if(uxQueueMessagesWaiting(MsgQueue)!=0)
    {
        if(xQueueReceive( MsgQueue, (void*)&receivenum ,  10 )== pdPASS )
        {
        	if(receivenum==1)
        	{
			#if MP3_CODEC_DEMO_PLAY_STOP_STEREOMONO_SR_8K_TO_48K == 1
				mp3_codec_demo_play_stop_StereoMono_SR_8K_to_48K();
			#endif
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
				printf("/******************************************************************/\r\n");
				if(res==1)mp3_codec_demo_play_stop();

			#if 1
			//  vTaskDelay(5000 / portTICK_RATE_MS);
				while (1) {
					vTaskDelay(100 / portTICK_RATE_MS);
					//vTaskDelay(10000);
				}
			#endif
        	}
            printf("ReceiveNum:%d\r\n",receivenum);
        }
    }
    vTaskDelay(1000);
	}
}

