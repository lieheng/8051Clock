#include "DS18B20.h"

unsigned char code Array_Point[] = {0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9}; // 小数查表

void DS18B20_Init()
{
    unsigned char x = 0;
    DQ = 1; // DQ复位
    DS18B20_Delay(10);
    // Delay(8);  //稍做延时,10us
    DQ = 0; // 单片机将DQ拉低
    DS18B20_Delay(500);
    // Delay(80); //精确延时 大于 480us ,498us
    DQ = 1; // 拉高总线
    DS18B20_Delay(154);
    // Delay(14);	//154us
    x = DQ; // 稍做延时后 如果x=0则初始化成功 x=1则初始化失败
    DS18B20_Delay(212);
    // Delay(20); //212us
}

unsigned char DS18B20_ReadOneChar()
{
    unsigned char i = 0;
    unsigned char dat = 0;
    for (i = 8; i > 0; i--)
    {
        DQ = 0; // 给脉冲信号
        dat >>= 1;
        DQ = 1; // 给脉冲信号
        if (DQ)
            dat |= 0x80;
        DS18B20_Delay(56);
        // Delay(4); //56us
    }
    return (dat);
}

void DS18B20_WriteOneChar(unsigned char dat)
{
    unsigned char i = 0;
    for (i = 8; i > 0; i--)
    {
        DQ = 0;
        DQ = dat & 0x01;
        DS18B20_Delay(66);
        // Delay(5); //66us
        DQ = 1;
        dat >>= 1;
    }
}

unsigned int DS18B20_ReadTemperature(void)
{
    unsigned char a = 0;
    unsigned char b = 0;
    unsigned int t = 0;
    DS18B20_Init();
    DS18B20_WriteOneChar(0xCC); // 跳过读序号列号的操作
    DS18B20_WriteOneChar(0x44); // 启动温度转换
    DS18B20_Init();
    DS18B20_WriteOneChar(0xCC); // 跳过读序号列号的操作
    DS18B20_WriteOneChar(0xBE); // 读取温度寄存器等（共可读9个寄存器） 前两个就是温度
    a = DS18B20_ReadOneChar();
    b = DS18B20_ReadOneChar();
    t = b;
    t <<= 8;
    t = t | a;
    // t= t/2; //放大10倍输出并四舍五入---此行没用
    return (t);
}

void DS18B20_Delay(unsigned int uiUs) // us延时函数
{
    for (; uiUs > 0; uiUs--)
    {
        ;
    }
}