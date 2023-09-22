#include <REG51.H>

#ifndef __AT24C02_H__
#define __AT24C02_H__

/*********************IIC.h*************************************/
//--����ʹ�õ�IO��--//
sbit I2C_SCL = P2 ^ 1;
sbit I2C_SDA = P2 ^ 0;

//--����ȫ�ֱ���--//
void I2C_Start();                    // ��ʼ�źţ���I2C_SCLʱ���ź��ڸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ���½���
void I2C_Stop();                     // ��ֹ�źţ���I2C_SCLʱ���źŸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ��������
bit I2C_SendByte(unsigned char dat); // ʹ��I2c��ȡһ���ֽ�
unsigned char I2C_ReadByte();        // ͨ��I2C����һ���ֽڡ���I2C_SCLʱ���źŸߵ�ƽ�ڼ䣬���ַ����ź�I2C_SDA�����ȶ�

void AT24C02_Write(unsigned char addr, unsigned char dat); // ��AT24C02��д��һ���ֽ�
unsigned char AT24C02_Read(unsigned char addr);            // ��AT24C02�ж���һ���ֽ�

#endif