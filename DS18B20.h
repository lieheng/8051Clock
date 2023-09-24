#include <REG51.H>

#ifndef __DS18B20_H__
#define __DS18B20_H__

sbit DQ = P0 ^ 0; // 温度传感器引脚

extern unsigned char code Array_Point[]; // 小数查表

void DS18B20_Init();
unsigned char DS18B20_ReadOneChar();
void DS18B20_WriteOneChar(unsigned char dat);
unsigned int DS18B20_ReadTemperature(void);
void DS18B20_Delay(unsigned int uiUs); // us延时函数

#endif