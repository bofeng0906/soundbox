/*
 * mp3_synfile.c
 *
 *  Created on: 2018年6月26日
 *      Author: ydg
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
//
#include <ff.h>
#include <mp3_synfile.h>
/////////////////////////////////////////////////////////////////////////////////////
const char weight[6]="WEIGHT";
const char kilogram[8] ="KILOGRAM";
const char digital[4] ="NUM_";
uint8_t weightsynflag=1;//1:合成完成 0：未完成
uint8_t kilogramsynflag=1;
uint8_t digitalsynflag=1;
/////////////////////////////////////////////////////////////////////////////////////
uint16_t tempvalue,num=0;
uint8_t SynMp3files (char* path,uint16_t value)
{
    FILINFO fno;
    DIR dir;
	uint8_t i,j;
	uint8_t res;
    char PathBuf[50];
	char tempchar[5]={0,0,0,0,0};
	char digitalbuf[50]="NUM_";

    num=0;
	tempvalue=value;
	while(tempvalue)
	{
       num++;//数字位数
       tempvalue/=10;
    }
	sprintf(tempchar, "%d" , value);
	strncat(digitalbuf,tempchar,1);
	strcat(digitalbuf,".MP3");
	weightsynflag=0;
	num--;
	f_unlink("SD:/temp/synfile.mp3");//删除文件
	printf("[MP3 Syn] mp3_synfile start!\r\n");
	while(1)
	{
		res = f_opendir(&dir, path);      /* Open the directory */
		if (res == FR_OK)
		{
			for (;;)
			{
				res = f_readdir(&dir, &fno);
//				if (res != FR_OK || fno.fname[0] == 0)  			/* Break on error or end of dir */
				if (fno.fname[0] == 0)  							/* Break on error or end of dir */
				{
				   f_closedir(&dir);
				   f_opendir(&dir, path);
				}

				if ((fno.fname[0] == '.')||((fno.fattrib & AM_DIR)!=0))
					continue;																							/* Ignore dot entry and directory*/
				for(i=0;i<13;i++)
				{
					if(fno.fname[i]=='.')
						break;
				}
				if(i>8)
					continue;

				if ((((fno.fname[i+1] == 'm')||(fno.fname[i+1] == 'M'))
					&&((fno.fname[i+2] == 'p')||(fno.fname[i+2] == 'P'))
				&&(fno.fname[i+3] == '3')))
				{
					if((strncmp((const char*) weight,fno.fname,sizeof(weight))==0)&&(weightsynflag==0))//先合成体重音频文件
					{
						  strcpy(PathBuf,path);
						  strcat(PathBuf,"/");
						  strcat(PathBuf,fno.fname);
						  JointfMp3File(PathBuf);
						  printf("[MP3 Syn] weightsynflag ok!\r\n");
						  weightsynflag=1;
						  digitalsynflag=0;
					}
					else if((strncmp((const char*) fno.fname,digitalbuf,9)==0)&&(digitalsynflag==0))//合成NUM音频文件
					{
						  strcpy(PathBuf,path);
						  strcat(PathBuf,"/");
						  strcat(PathBuf,fno.fname);
						  JointfMp3File(PathBuf);

						  if(num--)
						  {
								for(uint8_t k=0;k<5;k++)
								{
									   tempchar[k]=tempchar[k+1];
							    }
								memset(digitalbuf, 0, sizeof(digitalbuf));
								strcpy(digitalbuf,"NUM_");
			                    strncat(digitalbuf,tempchar,1);
							    strcat(digitalbuf,".MP3");
						  }else
						  {
							    printf("[MP3 Syn] digitalsynflag ok!\r\n");
						        digitalsynflag=1;
		                        kilogramsynflag=0;
						  }
					}
					else if((strncmp((const char*) kilogram,fno.fname,sizeof(kilogram))==0)&&(kilogramsynflag==0))//播放KG音频文件
					{
						  strcpy(PathBuf,path);
						  strcat(PathBuf,"/");
						  strcat(PathBuf,fno.fname);
						  JointfMp3File(PathBuf);
					      kilogramsynflag=1;
					      printf("[MP3 Syn] kilogramsynflag ok!\r\n");
						  break;
					}
				}
			}
			f_closedir(&dir);
		}else
		{
			printf("[MP3 Syn] Fail to open synfile path!\r\n");
			res=0;
			break;
		}
		if(kilogramsynflag)
		{
			res=1;
			break;
		}
	}
    return res;
}
///////////////////////////////////////////////////////////////////////
#define datasize  1024*10
uint8_t TempBuf[datasize];
uint8_t FileBuf[datasize];
uint32_t comfileflag=0;
uint32_t tagsize=0;
uint32_t Mp3Framelength=0;
uint32_t ptr=0;
uint32_t tempptr=0;
int32_t  buffOffset;
FIL Mp3File;
FILINFO audiofno;
void JointfMp3File(char* path)
{
     uint8_t  res=0;
	 uint32_t br=0,bw;
	 uint32_t Mp3DataStart=0;
//	 ID3V2_TagHead *TagHead;
//	 MP3FrameInfo Mp3FrameInfo;

	 ptr=0;
	 tempptr=0;
	 comfileflag=0;

	 res=f_open(&Mp3File, "SD:/temp/synfile.mp3", FA_WRITE);
	 if(res!=FR_OK)
	 {
		  res=f_open(&Mp3File, "SD:/temp/synfile.mp3", FA_CREATE_ALWAYS);
		  if(res==FR_OK)
		  {
			 f_close(&Mp3File);
		  }
	 }else
	 {
		  f_close(&Mp3File);
		  f_stat("SD:/temp/synfile.mp3", &audiofno);
	 }

	 while(1)
	 {
		  res=f_open(&Mp3File,(TCHAR*)path,FA_READ);
		  if(res==FR_OK)
		  {
			    res=f_read(&Mp3File,TempBuf,datasize,&br);
		  }
		  if(res==FR_OK)//成功读取音频数据
		  {
			    printf("[MP3 Syn] read mp3 audio data ok!\r\n");
				for(uint16_t i=0;i<br;i++)
				{
					FileBuf[i]=TempBuf[i];
				}
				if(br>=datasize)
				{
					f_close(&Mp3File);
					res = f_open(&Mp3File, "SD:/temp/synfile.MP3", FA_WRITE);
					if(res != FR_OK)
					{

					}
					else
					{
						  res = f_lseek(&Mp3File, audiofno.fsize);
						  if(res==FR_OK)
						  {
							  res = f_write(&Mp3File, FileBuf, br, &bw);   /* Write it to the dst file */
						  }
					}
					f_close(&Mp3File);
					res=f_read(&Mp3File,TempBuf,datasize,&br);
					for(uint16_t i=0;i<br;i++)
					{
						 FileBuf[i]=TempBuf[i];
					}
					res = f_open(&Mp3File, "SD:/temp/synfile.MP3", FA_WRITE);
					if(res != FR_OK)
					{

					}
					else
					{
						 res = f_lseek(&Mp3File, audiofno.fsize);
						 if(res==FR_OK)
						 {
							 res = f_write(&Mp3File, FileBuf, br, &bw);               /* Write it to the dst file */
						 }
					}
					f_close(&Mp3File);
				}else
				{
					f_close(&Mp3File);
					res = f_open(&Mp3File, "SD:/temp/synfile.MP3", FA_WRITE);
					if(res != FR_OK)
					{

					}
					else
					{
						 res = f_lseek(&Mp3File, audiofno.fsize);
						 if(res==FR_OK)
						 {
							  res = f_write(&Mp3File, FileBuf, br, &bw);               /* Write it to the dst file */
						 }
					}
					f_close(&Mp3File);
				}
		  }
//		  f_close(&Mp3File);



		  if(res==FR_OK)
		  {
		  	  break;
		  }
	 }
}

