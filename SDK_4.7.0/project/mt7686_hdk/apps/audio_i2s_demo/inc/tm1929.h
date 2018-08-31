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

/*********************************************************/
#define	RSTCTR 	       0x00		     // 器件地址
#define	RGB_OE 	       0x01		     // 器件地址
#define	FIXBRIT_LED0 	 0x02		     // 器件地址
#define	FIXBRIT_LED1 	 0x03		     // 器件地址
#define	FIXBRIT_LED2 	 0x04		     // 器件地址
#define	FIXBRIT_LED3 	 0x05		     // 器件地址
#define	FIXBRIT_LED4 	 0x06		     // 器件地址
#define	FIXBRIT_LED5 	 0x07		     // 器件地址
#define	FIXBRIT_LED6 	 0x08		     // 器件地址
#define	FIXBRIT_LED7 	 0x09		     // 器件地址
#define	FIXBRIT_LED8 	 0x0A		     // 器件地址
#define	FIXBRIT_LED9 	 0x0B		     // 器件地址
#define	FIXBRIT_LED10 	 0x0C		   // 器件地址
#define	FIXBRIT_LED11 	 0x0D		   // 器件地址
#define	FIXBRIT_LED12    0x0E		   // 器件地址
#define	FIXBRIT_LED13 	 0x0F		   // 器件地址
#define	FIXBRIT_LED14 	 0x10		   // 器件地址
#define	FIXBRIT_LED15 	 0x11		   // 器件地址
#define	FIXBRIT_LED16 	 0x12		   // 器件地址
#define	FIXBRIT_LED17 	 0x13		   // 器件地址

#define LED1929_I2C_ADDR                               0x46

#define PWM_LEN   255

int8_t led1929_i2c_write(uint8_t u8Addr,uint8_t u8Data);
uint8_t led1929_i2c_read(uint8_t u8Addr);
void led1929_init(void);
void test_led1929(void);




