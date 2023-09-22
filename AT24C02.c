#include "AT24C02.h"

void AT24C02_Delay(int x)
{
	x /= 2;
	while (x--)
		;
}

// 起始信号：在I2C_SCL时钟信号在高电平期间I2C_SDA信号产生一个下降沿
void I2C_Start()
{
	I2C_SDA = 1;
	I2C_SCL = 1;
	AT24C02_Delay(5); // 建立时间是I2C_SDA保持时间>4.7us
	I2C_SDA = 0;
	AT24C02_Delay(5); // 保持时间是>4us
	I2C_SCL = 0;
}

// 终止信号：在I2C_SCL时钟信号高电平期间I2C_SDA信号产生一个上升沿
void I2C_Stop()
{
	I2C_SDA = 0;
	I2C_SCL = 1;
	AT24C02_Delay(5); // 建立时间大于4.7us
	I2C_SDA = 1;
}

// 通过I2C发送一个字节。在I2C_SCL时钟信号高电平期间， 保持发送信号I2C_SDA保持稳定
bit I2C_SendByte(unsigned char dat)
{
	unsigned char i = 0;
	unsigned char wait = 255; // 最大255，一个机器周期为1us，最大延时255us。
	for (i = 0; i < 8; i++)	  // 要发送8位，从最高位开始
	{
		if (dat & 0x80)
			I2C_SDA = 1; // 起始信号之后I2C_SCL=0，所以可以直接改变I2C_SDA信号
		else
			I2C_SDA = 0;
		dat <<= 1;
		AT24C02_Delay(5); // 建立时间>4.7us
		I2C_SCL = 1;
		AT24C02_Delay(5); // 建立时间>4.7us
		I2C_SCL = 0;
		AT24C02_Delay(5); // 时间大于4us
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

// 使用I2c读取一个字节
unsigned char I2C_ReadByte()
{
	unsigned char i = 0, dat = 0;
	I2C_SDA = 1; // 起始和发送一个字节之后I2C_SCL都是0
	AT24C02_Delay(5);
	for (i = 0; i < 8; i++) // 接收8个字节
	{
		I2C_SCL = 1;
		AT24C02_Delay(5);
		dat <<= 1;
		if (I2C_SDA)
			dat |= 1; // 接收到的数据最高位存放在dat的最低位
		I2C_SCL = 0;
		AT24C02_Delay(5);
	}
	return dat;
}

void AT24C02_Write(unsigned char addr, unsigned char dat)
{
	I2C_Start();
	I2C_SendByte(0xa0); // 发送写器件地址
	I2C_SendByte(addr); // 发送要写入内存地址
	I2C_SendByte(dat);	// 发送数据
	I2C_Stop();
	AT24C02_Delay(5000);
}

unsigned char AT24C02_Read(unsigned char addr)
{
	unsigned char num;
	I2C_Start();
	I2C_SendByte(0xa0); // 发送写器件地址
	I2C_SendByte(addr); // 发送要读取的地址
	I2C_Start();
	I2C_SendByte(0xa1);	  // 发送读器件地址
	num = I2C_ReadByte(); // 读取数据
	I2C_Stop();
	AT24C02_Delay(5000);

	return num;
}