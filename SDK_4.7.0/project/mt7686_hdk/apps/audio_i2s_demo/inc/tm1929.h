/******************************************************************************
//*��Ȩ��Ϣ����������΢�������޹�˾
//*�� �� ����TM1929-V1.0
//*��ǰ�汾��V1.0
//*MCU �ͺţ�STC12C5608AD
//*����������Keil uVision4
//*����Ƶ�ʣ�33MHZ(�ڲ�ʱ��)       
//*������ڣ�2018-04-10
//*�����ܣ�1.�ó����ʵ��TM1929�׵����ε�����
//*����������1.�˳���ΪTM1929����LED������ʾ���򣬽����ο�֮�á�
//           2.����ֱ��ʹ�ñ����̳�����ɾ�����ʧ�ģ�����˾���е��κ����Ρ�             
/*******************************************************************************/

/*********************************************************/
#define	RSTCTR 	       0x00		     // ������ַ
#define	RGB_OE 	       0x01		     // ������ַ
#define	FIXBRIT_LED0 	 0x02		     // ������ַ
#define	FIXBRIT_LED1 	 0x03		     // ������ַ
#define	FIXBRIT_LED2 	 0x04		     // ������ַ
#define	FIXBRIT_LED3 	 0x05		     // ������ַ
#define	FIXBRIT_LED4 	 0x06		     // ������ַ
#define	FIXBRIT_LED5 	 0x07		     // ������ַ
#define	FIXBRIT_LED6 	 0x08		     // ������ַ
#define	FIXBRIT_LED7 	 0x09		     // ������ַ
#define	FIXBRIT_LED8 	 0x0A		     // ������ַ
#define	FIXBRIT_LED9 	 0x0B		     // ������ַ
#define	FIXBRIT_LED10 	 0x0C		   // ������ַ
#define	FIXBRIT_LED11 	 0x0D		   // ������ַ
#define	FIXBRIT_LED12    0x0E		   // ������ַ
#define	FIXBRIT_LED13 	 0x0F		   // ������ַ
#define	FIXBRIT_LED14 	 0x10		   // ������ַ
#define	FIXBRIT_LED15 	 0x11		   // ������ַ
#define	FIXBRIT_LED16 	 0x12		   // ������ַ
#define	FIXBRIT_LED17 	 0x13		   // ������ַ

#define LED1929_I2C_ADDR                               0x46

#define PWM_LEN   255

int8_t led1929_i2c_write(uint8_t u8Addr,uint8_t u8Data);
uint8_t led1929_i2c_read(uint8_t u8Addr);
void led1929_init(void);
void test_led1929(void);




