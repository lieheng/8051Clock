#include <REG51.H>

#ifndef __AT24C02_H__
#define __AT24C02_H__

/*********************IIC.h*************************************/
//--定义使用的IO口--//
sbit I2C_SCL = P2 ^ 1;
sbit I2C_SDA = P2 ^ 0;

//--声明全局变量--//
void I2C_Start();                    // 起始信号：在I2C_SCL时钟信号在高电平期间I2C_SDA信号产生一个下降沿
void I2C_Stop();                     // 终止信号：在I2C_SCL时钟信号高电平期间I2C_SDA信号产生一个上升沿
bit I2C_SendByte(unsigned char dat); // 使用I2c读取一个字节
unsigned char I2C_ReadByte();        // 通过I2C发送一个字节。在I2C_SCL时钟信号高电平期间，保持发送信号I2C_SDA保持稳定

void AT24C02_Write(unsigned char addr, unsigned char dat); // 向AT24C02中写入一个字节
unsigned char AT24C02_Read(unsigned char addr);            // 从AT24C02中读出一个字节

#endif