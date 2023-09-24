#include <REG51.H>

#ifndef __LCD12864_H__
#define __LCD12864_H__

sbit LCD12864_RS = P3 ^ 4;
sbit LCD12864_RW = P3 ^ 5;
sbit LCD12864_EN = P3 ^ 6;
sbit LCD12864_PSB = P3 ^ 7;

#define LCDPORT P1

#define LINE0 0x80
#define LINE1 0x90
#define LINE2 0x88
#define LINE3 0x98

void LCD12864_Init(void);
void LCD12864_WriteInfomation(unsigned char ucData, bit bComOrData);
void LCD12864_CheckBusy(void);
void LCD12864_DisplayOneLine(unsigned char ucPos, unsigned char *ucStr);
void LCD12864_Delay(unsigned int uiCount);

#endif