#include "LCD12864.h"

void LCD12864_WriteInfomation(unsigned char ucData, bit bComOrData)
{
    LCD12864_CheckBusy();
    LCD12864_RW = 0;
    LCD12864_RS = bComOrData;
    LCD12864_EN = 1;
    LCDPORT = ucData;
    LCD12864_Delay(5);
    LCD12864_EN = 0;
}

void LCD12864_Init(void)
{
    LCD12864_PSB = 1;

    LCD12864_WriteInfomation(0x30, 0);
    LCD12864_WriteInfomation(0x0c, 0);
    LCD12864_WriteInfomation(0x01, 0);
    LCD12864_WriteInfomation(0x06, 0);
}

void LCD12864_CheckBusy(void)
{
    unsigned char i = 250;
    LCD12864_RS = 0;
    LCD12864_RW = 1;
    LCD12864_EN = 1;
    while ((i > 0) && (P1 & 0x80))
        i--;
    LCD12864_EN = 0;
}

void LCD12864_DisplayOneLine(unsigned char position, unsigned char *p)
{
    unsigned char i;
    switch (position)
    {
    case 0:
        LCD12864_WriteInfomation(LINE0, 0);
        break;
    case 1:
        LCD12864_WriteInfomation(LINE1, 0);
        break;
    case 2:
        LCD12864_WriteInfomation(LINE2, 0);
        break;
    case 3:
        LCD12864_WriteInfomation(LINE3, 0);
        break;
    default:
        break;
    }

    for (i = 0; i < 16; i++)
    {
        LCD12864_WriteInfomation(*p, 1);
        p++;
    }
}

void LCD12864_Delay(unsigned int uiCount)
{
    uiCount /= 2;
    while (uiCount--)
        ;
}