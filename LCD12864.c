#include "LCD12864.h"

/******************************************************************************
函数名称：LCD12864_WriteInfomation
函数功能：向LCD12864写入命令或者数据
入口参数：ucData-要写入液晶的数据或者命令的内容
          bComOrData-命令或者数据的标志位选择，0或者1，其中
            1：写入的是数据
            0：写入的是命令
返回值：无
备注：无
*******************************************************************************/
void LCD12864_WriteInfomation(unsigned char ucData, bit bComOrData)
{
    LCD12864_CheckBusy();     // 忙检测
    LCD12864_RW = 0;          // 拉低RW
    LCD12864_RS = bComOrData; // 根据标志位判断写入的是命令还是数据
    LCD12864_EN = 1;          // 使能信号
    LCDPORT = ucData;         // 将数据送至数据端口
    LCD12864_Delay(5);        // 延时
    LCD12864_EN = 0;          // 按照时序来操作
}

/******************************************************************************
函数名称：LCD12864_Init
函数功能：LCD12864液晶初始化
入口参数：无
返回值：无
备注：无
*******************************************************************************/
void LCD12864_Init(void)
{
    LCD12864_PSB = 1; // 8位并口工作模式

    LCD12864_WriteInfomation(0x30, 0); // 基本指令集
    LCD12864_WriteInfomation(0x0c, 0); // 游标设置
    LCD12864_WriteInfomation(0x01, 0); // 清屏
    LCD12864_WriteInfomation(0x06, 0); // 进入点设定
}

/******************************************************************************
函数名称：LCD12864_CheckBusy
函数功能：忙检测
入口参数：无
返回值：无
备注：使用变量i做计时，避免液晶在死循环处停滞。
*******************************************************************************/
void LCD12864_CheckBusy(void)
{
    unsigned char i = 250; // 局部变量
    LCD12864_RS = 0;       // 拉低
    LCD12864_RW = 1;       // 拉高
    LCD12864_EN = 1;       // 使能
    while ((i > 0) && (P1 & 0x80))
        i--;         // 判断忙标志位
    LCD12864_EN = 0; // 释放
}

/******************************************************************************
函数名称：LCD12864_DisplayOneLine
函数功能：显示一行汉字（8个汉字或者16个英文字符）
入口参数：position-要显示的行的首地址，可选值0x80,0x88,0x90,0x98,其中：
            0x80:液晶的第一行；
            0x88:液晶的第三行；
            0x90:液晶的第二行；
            0x98:液晶的第四行。
          p-要显示的内容的首地址。
返回值：无
备注：无
*******************************************************************************/
void LCD12864_DisplayOneLine(unsigned char position, unsigned char *p, unsigned char len)
{
    unsigned char i;
    LCD12864_WriteInfomation(position, 0); // 写入要显示文字的行的首地址
    // LCD12864_Delay(150);

    for (i = 0; i < len; i++) // 依次执行写入操作
    {
        LCD12864_WriteInfomation(*p, 1);
        p++;
    }
}

/******************************************************************************
函数名称：Delay
函数功能：延时函数
入口参数：uiCount-延时参数
返回值：无
备注：无
*******************************************************************************/
void LCD12864_Delay(unsigned int uiCount)
{
    uiCount /= 2;
    while (uiCount--)
        ;
}