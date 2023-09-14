#include <REG51.H>

#define INTERVAL 1 // 间隔1毫秒

#define INIT_HOUR 23   // 初始化小时
#define INIT_MINUTE 59 // 初始化分钟
#define INIT_SECOND 20 // 初始化秒

unsigned char hour = INIT_HOUR;     // 初始化小时
unsigned char minute = INIT_MINUTE; // 初始化分钟
unsigned char second = INIT_SECOND; // 初始化秒

unsigned char displayIndex = 0;
unsigned char LED8[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned int interruptCount = 0; // 中断次数

sbit SEG_DS = P2 ^ 0;   // 74HC595芯片的数据引脚
sbit SEG_SHCP = P2 ^ 1; // 74HC595芯片的控制引脚，上升沿移入数据
sbit SEG_STCP = P2 ^ 2; // 74HC595芯片的控制引脚，上升沿更新数据

unsigned char code Seg_Data[] = {
    // 共阳数码管的编码，并将数据定义在CODE区
    0xc0, /*0*/
    0xF9, /*1*/
    0xA4, /*2*/
    0xB0, /*3*/
    0x99, /*4*/
    0x92, /*5*/
    0x82, /*6*/
    0xF8, /*7*/
    0x80, /*8*/
    0x90, /*9*/
    0x88, /*A*/
    0x83, /*b*/
    0xC6, /*C*/
    0xA1, /*d*/
    0x86, /*E*/
    0x8E, /*F*/
    0xBF, /*-*/
    0xFF, /*OFFF*/
};

unsigned char code Seg_Addr[] = {
    // 数码管位选编码，控制显示8位中的第几位
    0x01,
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80,
    0xFF, // ALL ON
    0x00  // OFF
};

void DisplayOneCharOnAddr(unsigned char, unsigned char Addr);
void SEG_Send595OneByte(unsigned char ucData); // 向74HC595写入一个8位的数据
void SecondIncrease();

void SecondIncrease()
{
    second++;
    if (second == 60)
    {
        second = 0;
        minute++;
    }
    if (minute == 60)
    {
        minute = 0;
        hour++;
    }
    if (hour == 24)
    {
        hour = 0;
    }
}

void Display()
{
    DisplayOneCharOnAddr(LED8[displayIndex], displayIndex); // 显示数码管
    displayIndex++;
    displayIndex %= 8; // 显示下一个数码管
}

void Timer0() interrupt 1
{ // 定时器0中断服务函数
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == 1000 / INTERVAL)
    { // 1秒
        interruptCount = 0;
        SecondIncrease();
    }

    LED8[0] = hour / 10; // 显示小时十位
    LED8[1] = hour % 10; // 显示小时个位
    LED8[2] = 16;
    LED8[3] = minute / 10; // 显示分钟十位
    LED8[4] = minute % 10; // 显示分钟个位
    LED8[5] = 16;
    LED8[6] = second / 10; // 显示秒十位
    LED8[7] = second % 10; // 显示秒个位

    Display();
}

void Init()
{
    EA = 1; // 开启总中断

    TMOD = 0x01; // 设置计时器0工作在方式1
    ET0 = 1;     // 允许计时器0中断
    TR0 = 1;     // 启动计时器
    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;
}

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

// 向HC595发送一个字节
void SEG_Send595OneByte(unsigned char ucData)
{
    unsigned char i;

    for (i = 0; i < 8; i++) // 8位数据依次写入，先写最低位
    {
        SEG_DS = (ucData & 0x80); // 先读入高位   x&0x80;
        SEG_SHCP = 0;
        SEG_SHCP = 1;
        SEG_SHCP = 0; // SHCP引脚的上升沿移入数据
        ucData <<= 1; // 数据左移
    }
}

/*******************************************************
函数功能：在指定位置显示一个数据
参数说明：Data是要显示的数据，Addr是在第几位显示。

Addr取值范围是0~9。
Addr=0~7时，选择的是显示在第几位数码管上;
Addr=8  时，同时选中8位数码管，即打开所有数码管
Addr=9  时，关闭8位数码管

8位数码管，左数依次为第0位，第1位...第7位。

*******************************************************/
void DisplayOneCharOnAddr(unsigned char Data, unsigned char Addr)
{
    SEG_Send595OneByte(Seg_Addr[Addr]); // 显示在哪一个数码管上
    SEG_Send595OneByte(Seg_Data[Data]); // 显示的数据
    SEG_STCP = 0;
    SEG_STCP = 1; // STCP引脚的上升沿更新数据
    SEG_STCP = 0;
}
