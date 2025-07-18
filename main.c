#include <LPC22xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart0.h"
#include "delay.h"
#include "rtc.h"
#include "keypad.h"
#include "lcd_defines.h"
#include "lcd.h"
#include "I2C_EEPROM.h"
#include "I2C.h"
#include "types.h"
#include "pin_cfg.h"
#include "pin_cfg_defines.h"
#include "defines.h"


#define EN1 21
#define PIN1 22
#define PIN2 23
//#define Buzzer 24
#define Time 2

#define EINT3_PIN      20 //select the one port pin which will give support for EINT3
#define EINT3_VIC_CHNO 17         //assign the EINT3 channel number
#define EINT3_LED      25       //toggle one LED for every interrupt, so select one port pin


unsigned char pwd[5]="1234",r_pwd[5];

char str[7];
char i =0;
u32 Hr,Mn,Sec,start_sec,start_min;
u32 Key,Key1,Key2,Key3,Key4,Key5,Key6;

void otp(u32 num)
{
        if(num==0)
        {
                //display 0 on lcd
                str[i++] = '0';
        }
        else
        {
                //extract digit by digit and store it into array
                while(num)
                {
                        str[i++] = (num%10)+48;
                        num = num/10;
                }
                //display digit by digit on LCD
        }
}

void GSM_Send_SMS(char *number,char *msg)
{
   UART0_TxString("AT\r\n");
   delay_ms(1000);
   UART0_TxString("AT+CMGF=1\r\n");
   delay_ms(1000);
   UART0_TxString("AT+CMGS=");
   UART0_TxChar('"');
   UART0_TxString(number);
   UART0_TxChar('"');
   UART0_TxString("\r\n");
   delay_ms(1000);
   //UART0_TxString("Do Not Share this OTP with anyone");
   UART0_TxString(msg);
   UART0_TxChar(0x1A);// Ctrl+z to send sms
   delay_ms(5000);
}

//ISR SYNTAX: void ISR_NAME(void) __irq
void eint0_isr(void) __irq
{
        //toggle EINT_LED
        CPLBIT(IOPIN0,EINT3_LED);
        CmdLCD(CLEAR_LCD);
        StrLCD("old password");
        Key5=ReadNum();
        if(Key5==Key3)
        {
            CmdLCD(CLEAR_LCD);
           StrLCD("New PASSWORD- ");
           Key4=ReadNum();
					 CmdLCD(CLEAR_LCD);
					 StrLCD("confirm pass");
	         Key6=ReadNum();
					if(Key4==Key6)
					{
           delay_ms(100);
           pwd[0]=((Key4%10000)/1000)+48;
					 pwd[1]=((Key4%1000)/100)+48;
					 pwd[2]=((Key4%100)/10)+48;
					 pwd[3]=(Key4%10)+48;
          }
					else
					{
					CmdLCD(CLEAR_LCD);
					StrLCD("wrong pass");
					delay_ms(200);
					}
					//i2c_eeprom_page_write(0x50,0x0000,pwd,4);
					//i2c_eeprom_seq_read(0x50,0x0000,r_pwd,4);

					//Key3=atoi((const char *)r_pwd);
          Key3=atoi((const char *)pwd);
        }
        else
        {
					CmdLCD(CLEAR_LCD);
					StrLCD("wrong password");
					delay_ms(200);
        }
				delay_ms(10);
        CmdLCD(0x01);
				delay_ms(10);
				StrLCD("1.ENTER PASSWORD");
				CmdLCD(0xc1);
				//StrLCD("1");
        //clear eint0 status (through EXTINT SFR)
        EXTINT = 1<<3;
        //clear eint0 status in VIC (through VICVectAddr )
        VICVectAddr = 0;

}

void Enable_EINT0(void)
{
        //configure P0.20 as EINT3 input pin
        CfgPortPin(PORT0,EINT3_PIN,PIN_FUNC4);

        //configure VIC (Vector Interrupt controller)
        //def External interrupts (EINT0/EINT1/EINT2/EINT3) is IRQ types (by using VICIntSelect SFR)

        //enable EINT0 via VIC (by using VICIntEnable SFR)
        VICIntEnable = 1<<EINT3_VIC_CHNO;
        //enable vectored irq slot 0 for EINT0 (0-15 based on requirement <96> by using VICVectCntl0 SFR)
        VICVectCntl0 = (1<<5) | (EINT3_VIC_CHNO);
        //load isr address into slot 0 (0-15 based on requirement <96> by using VICVectAddr0 SFR)
        VICVectAddr0 = (u32)eint0_isr;

        //configure External Interrupts Peripheral
        //configure EINT0/EINT1/EINT2/EINT3 as edge triggered (use EXTMODE SFR)
        EXTMODE = 1<<3;
        //def EINT0/EINT1/EINT2/EINT3 is Falling Edge Triggerd (use EXTPOLAR SFR)
}


int main()
{
        InitLCD();
        Keypad_Init();
        Enable_EINT0();
        //init_i2c();
         RTC_Init();
	      //UART0_Init();
       RTCSetTime(1,15,5);

              IODIR0|=1<<EN1;
              IODIR0|=1<<PIN1;
              IODIR0|=1<<PIN2;
              //IODIR0|=1<<Buzzer;

        IODIR0|=1<<EINT3_LED;

        IOSET0|=1<<EN1;

           //    i2c_eeprom_page_write(0x50,0x0000,pwd,4);
         //i2c_eeprom_seq_read(0x50,0x0000,r_pwd,4);

              //Key3=atoi((const char *)r_pwd);
              Key3=atoi((const char *)pwd);

while(1){

	

        CmdLCD(0x01);// clear lcd
        CmdLCD(0x0c);// cursor off
        CmdLCD(0x80);// on 1st line
              StrLCD("OTP Based Limited");
              CmdLCD(0xC0);
              StrLCD("Access System");
        delay_ms(200);
        ip:
              CmdLCD(CLEAR_LCD);
                                StrLCD("1.ENTER PASSWORD");
        Key=ReadNum();
        //CmdLCD(0xC0);//on 2nd line
        delay_ms(20);
        //U32LCD(Key);


        if(Key==Key3)
        {
             CmdLCD(0x01);//clear lcd
             CmdLCD(0x80);
             StrLCD("Successfull");
                                           delay_ms(500);
                                           CmdLCD(CLEAR_LCD);
                   StrLCD("Sending SMS...");
             



             /*RTCGetTime(&Hr,&Mn,&Sec);
					   otp(Hr);
             otp(Mn);
             otp(Sec);*/
            otp(HOUR);
             otp(MIN);
             otp(SEC);

                                       // i=0;
            // GSM_Send_SMS("9516253166",str);
                                           delay_ms(100);
             ip1:

                                          CmdLCD(0x01);
                  Key2=atoi(str);
                  StrLCD("OTP: ");
                  SetCursor(1,12);
                //  CharLCD(':');
                                                                        //CharLCD(' ');
                                                                        U32LCD(Key2);
                //  U32LCD(Time);
                                                 RTCGetTime(&Hr,&Mn,&Sec);
                                                 start_sec=Sec;
                                                 start_min=Mn;
                                                 Key1=readNum();
                                                 RTCGetTime(&Hr,&Mn,&Sec);
                                                 //if(((Mn-start_min)*60 +(Sec-start_sec))>Time){
																									  if(((Sec-start_sec))>Time){
																													CmdLCD(0xc0);
                                                         StrLCD("Time Lapsed....");
                                                         delay_ms(700);
                                                         goto ip;
																			
                                                 }
             CmdLCD(0x01);
             delay_ms(20);
             U32LCD(Key1);
                                                 delay_ms(700);
             if(Key1==Key2)
             {
                 CmdLCD(0x01);
                 CmdLCD(0x80);
                 StrLCD("Log IN ACCESSED");

                                                           IOSET0|=1<<PIN2;
                                                           IOCLR0|=1<<PIN1;
                                                           delay_ms(1000);

                                                           IOSET0|=1<<PIN1;
                                                           IOSET0|=1<<PIN2;
                                                //         delay_ms(4000);
             }
             else
             {
                 CmdLCD(0x01);
                 CmdLCD(0x80);
                 StrLCD("Denied");
                        //                                 IOSET0|=1<<Buzzer;
                        //                                 delay_ms(1000);
                        //                                 IOCLR0|=1<<Buzzer;
                 goto ip1;
             }
        }
        else
        {
             CmdLCD(0x01);
             CmdLCD(0x80);
             StrLCD("Wrong Passcode");
             delay_ms(500);
             CmdLCD(0x01);
             goto ip;
        }
			}               }

			
			
			
			
			
			
			
			
