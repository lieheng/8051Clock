#include "AT24C02.h"

void AT24C02_Delay(int x)
{
	x /= 2;
	while (x--)
		;
}

// ��ʼ�źţ���I2C_SCLʱ���ź��ڸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ���½���
void I2C_Start()
{
	I2C_SDA = 1;
	I2C_SCL = 1;
	AT24C02_Delay(5); // ����ʱ����I2C_SDA����ʱ��>4.7us
	I2C_SDA = 0;
	AT24C02_Delay(5); // ����ʱ����>4us
	I2C_SCL = 0;
}

// ��ֹ�źţ���I2C_SCLʱ���źŸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ��������
void I2C_Stop()
{
	I2C_SDA = 0;
	I2C_SCL = 1;
	AT24C02_Delay(5); // ����ʱ�����4.7us
	I2C_SDA = 1;
}

// ͨ��I2C����һ���ֽڡ���I2C_SCLʱ���źŸߵ�ƽ�ڼ䣬 ���ַ����ź�I2C_SDA�����ȶ�
bit I2C_SendByte(unsigned char dat)
{
	unsigned char i = 0;
	unsigned char wait = 255; // ���255��һ����������Ϊ1us�������ʱ255us��
	for (i = 0; i < 8; i++)	  // Ҫ����8λ�������λ��ʼ
	{
		if (dat & 0x80)
			I2C_SDA = 1; // ��ʼ�ź�֮��I2C_SCL=0�����Կ���ֱ�Ӹı�I2C_SDA�ź�
		else
			I2C_SDA = 0;
		dat <<= 1;
		AT24C02_Delay(5); // ����ʱ��>4.7us
		I2C_SCL = 1;
		AT24C02_Delay(5); // ����ʱ��>4.7us
		I2C_SCL = 0;
		AT24C02_Delay(5); // ʱ�����4us
	}
	I2C_SDA = 1;
	AT24C02_Delay(5);
	I2C_SCL = 1;
	AT24C02_Delay(5);
	if (I2C_SDA)
	{
		I2C_SCL = 0;
		I2C_Stop();
		return 0;
	}
	else
	{
		I2C_SCL = 0;
		return 1;
	}
}

// ʹ��I2c��ȡһ���ֽ�
unsigned char I2C_ReadByte()
{
	unsigned char i = 0, dat = 0;
	I2C_SDA = 1; // ��ʼ�ͷ���һ���ֽ�֮��I2C_SCL����0
	AT24C02_Delay(5);
	for (i = 0; i < 8; i++) // ����8���ֽ�
	{
		I2C_SCL = 1;
		AT24C02_Delay(5);
		dat <<= 1;
		if (I2C_SDA)
			dat |= 1; // ���յ����������λ�����dat�����λ
		I2C_SCL = 0;
		AT24C02_Delay(5);
	}
	return dat;
}

void AT24C02_Write(unsigned char addr, unsigned char dat)
{
	I2C_Start();
	I2C_SendByte(0xa0); // ����д������ַ
	I2C_SendByte(addr); // ����Ҫд���ڴ��ַ
	I2C_SendByte(dat);	// ��������
	I2C_Stop();
	AT24C02_Delay(5000);
}

unsigned char AT24C02_Read(unsigned char addr)
{
	unsigned char num;
	I2C_Start();
	I2C_SendByte(0xa0); // ����д������ַ
	I2C_SendByte(addr); // ����Ҫ��ȡ�ĵ�ַ
	I2C_Start();
	I2C_SendByte(0xa1);	  // ���Ͷ�������ַ
	num = I2C_ReadByte(); // ��ȡ����
	I2C_Stop();
	AT24C02_Delay(5000);

	return num;
}