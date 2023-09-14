#include <REG51.H>

#define INTERVAL 1 // 间隔1毫秒

#define INIT_HOUR 23   // 初始化小时
#define INIT_MINUTE 59 // 初始化分钟
#define INIT_SECOND 55 // 初始化秒

#define HOURLYCHIMETIMES 3 // 整点响铃次数

unsigned char hour = INIT_HOUR;     // 初始化小时
unsigned char minute = INIT_MINUTE; // 初始化分钟
unsigned char second = INIT_SECOND; // 初始化秒

enum MODE
{
    SHOW,            // 显示模式
    SET,             // 设置模式
    SET_HOUR,        // 设置时模式
    SET_MINUTE,      // 设置分模式
    SET_SECOND,      // 设置秒模式
    STOPWATCH,       // 秒表模式
    STOPWATCH_START, // 秒表计时模式
    STOPWATCH_PAUSE, // 秒表暂停模式
    ALARMCLOCK       // 设置闹钟
};

unsigned char mode = SHOW; // 模式

unsigned char displayIndex = 0;
unsigned char LED8[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned int interruptCount = 0; // 中断次数

bit hourlyChime = 1; // 整点报时功能，1表示开，0表示关

sbit SEG_DS = P2 ^ 0;   // 74HC595芯片的数据引脚
sbit SEG_SHCP = P2 ^ 1; // 74HC595芯片的控制引脚，上升沿移入数据
sbit SEG_STCP = P2 ^ 2; // 74HC595芯片的控制引脚，上升沿更新数据

sbit Chime = P2 ^ 3; // 蜂鸣器

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

unsigned int checkCount = 0;
unsigned char shortOrLang = 0; // 0表示无效，1表示短按，2表示长按

bit buttonDown = 0;
bit button = 0;

sbit P3_2 = P3 ^ 2;
sbit P3_3 = P3 ^ 3;

// 判断按键是长按还是短按
void Check()
{
    checkCount++;
    if (checkCount < 10 / INTERVAL) // 10ms延迟去抖动
        return;

    if (checkCount > 1010 / INTERVAL) // 如果大于一秒，为长按
    {
        shortOrLang = 2;
        checkCount = 0;
        buttonDown = 0;
    }
    else
    {
        if ((button == 0 && P3_2 == 1) || (button == 1 && P3_3 == 1))
        {
            shortOrLang = 1;
            checkCount = 0;
            buttonDown = 0;
        }
    }
}

// 短按处理程序
void ShortPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SHOW:
            mode = SET;
            break;
        case SET:
            mode = STOPWATCH;
            break;
        case STOPWATCH:
            mode = SHOW;
            break;
        case SET_HOUR:
            mode = SET_MINUTE;
            break;
        case SET_MINUTE:
            mode = SET_SECOND;
            break;
        case SET_SECOND:
            mode = SET_HOUR;
            break;
        default:
            break;
        }
    }
    else
    {
        unsigned char setHour = LED8[0] * 10 + LED8[1];
        unsigned char setMinute = LED8[3] * 10 + LED8[4];
        unsigned char setSecond = LED8[6] * 10 + LED8[7];
        switch (mode)
        {
        case SET_HOUR:
            setHour++;
            setHour %= 24;
            LED8[0] = setHour / 10;
            LED8[1] = setHour % 10;
            break;
        case SET_MINUTE:
            setMinute++;
            setMinute %= 60;
            LED8[3] = setMinute / 10;
            LED8[4] = setMinute % 10;
            break;
        case SET_SECOND:
            setSecond++;
            setSecond %= 60;
            LED8[6] = setSecond / 10;
            LED8[7] = setSecond % 10;
            break;
        case STOPWATCH:
            mode = STOPWATCH_START;
            break;
        case STOPWATCH_START:
            mode = STOPWATCH_PAUSE;
            break;
        case STOPWATCH_PAUSE:
            mode = STOPWATCH_START;
            break;
        default:
            break;
        }
    }

    shortOrLang = 0;
}

// 长按处理程序
void LongPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SET:
            mode = SET_HOUR;
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            hour = LED8[0] * 10 + LED8[1];
            minute = LED8[3] * 10 + LED8[4];
            second = LED8[6] * 10 + LED8[7];

            mode = SHOW;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (mode)
        {
        case SHOW:
            hourlyChime = !hourlyChime;
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            mode = SHOW;
            break;
        case STOPWATCH_PAUSE:
            mode = STOPWATCH;
            break;
        default:
            break;
        }
    }

    shortOrLang = 0;
}

void Int0() interrupt 0
{
    if (buttonDown == 0)
    {
        buttonDown = 1;
        button = 0;
    }
}

void Int1() interrupt 2
{
    if (buttonDown == 0)
    {
        buttonDown = 1;
        button = 1;
    }
}

void SecondIncrease()
{
    second++;
    if (second > 59)
    {
        second = 0;
        minute++;
    }
    if (minute > 59)
    {
        minute = 0;
        hour++;
    }
    if (hour > 23)
    {
        hour = 0;
    }
}

void Display(unsigned char enable)
{
    enable >>= displayIndex;
    enable &= 1;
    if (enable == 1)
        DisplayOneCharOnAddr(LED8[displayIndex], displayIndex); // 显示数码管
    else
        DisplayOneCharOnAddr(17, displayIndex);
    displayIndex++;
    displayIndex %= 8; // 显示下一个数码管
}

unsigned char hourlyChimeTimes = 0;

unsigned int stopwatchMSecond = 0;
unsigned char stopwatchSecond = 0;
unsigned char stopwatchMinute = 0;

// 定时器0中断服务函数
void Timer0() interrupt 1
{

    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1秒
        interruptCount = 0;
        SecondIncrease();
    }

    if (buttonDown == 1)
    {
        Check();
    }

    if (shortOrLang == 1)
    {
        ShortPress();
    }
    else if (shortOrLang == 2)
    {
        LongPress();
    }

    if (mode == SHOW && hourlyChime == 1)
    {
        if (hourlyChimeTimes == 0 && minute == 0 && second == 0)
            hourlyChimeTimes = 2 * (HOURLYCHIMETIMES + 1); // 我也不知道这里为什么要加一，但事实就是它会响HOURLYCHIMETIMES-1次，所以要加一补上。
        if (hourlyChimeTimes != 0)
        {
            if (interruptCount > (500 / INTERVAL))
                Chime = !Chime;
            else
                Chime = 0;
            if (interruptCount % (500 / INTERVAL) == 0)
                hourlyChimeTimes--;
        }
    }

    switch (mode)
    {
    case SHOW:
        LED8[0] = hour / 10; // 显示小时十位
        LED8[1] = hour % 10; // 显示小时个位
        LED8[2] = 16;
        LED8[3] = minute / 10; // 显示分钟十位
        LED8[4] = minute % 10; // 显示分钟个位
        LED8[5] = 16;
        LED8[6] = second / 10; // 显示秒十位
        LED8[7] = second % 10; // 显示秒个位
        Display(0xFF);
        break;
    case SET:
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF);
        else
            Display(0); // 关闭数码管
        break;
    case SET_HOUR:
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF);
        else
            Display(0xFC);
        break;
    case SET_MINUTE:
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF);
        else
            Display(0xE7);
        break;
    case SET_SECOND:
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF);
        else
            Display(0x3F);
        break;
    case STOPWATCH:
        stopwatchMSecond = 0;
        stopwatchSecond = 0;
        stopwatchMinute = 0;
        LED8[0] = stopwatchMinute / 10; // 显示分钟十位
        LED8[1] = stopwatchMinute % 10; // 显示分钟个位
        LED8[2] = 16;
        LED8[3] = stopwatchSecond / 10; // 显示秒十位
        LED8[4] = stopwatchSecond % 10; // 显示秒个位
        LED8[5] = 16;
        LED8[6] = stopwatchMSecond / 100;
        LED8[7] = (stopwatchMSecond % 100) / 10;

        Display(0xFF);
        break;
    case STOPWATCH_START:
        stopwatchMSecond += INTERVAL;
        if (stopwatchMSecond > 999)
        {
            stopwatchMSecond = 0;
            stopwatchSecond++;
        }
        if (stopwatchSecond > 59)
        {
            stopwatchSecond = 0;
            stopwatchMinute++;
        }
        if (stopwatchMinute > 59)
        {
            stopwatchMinute = 0;
        }

        LED8[0] = stopwatchMinute / 10; // 显示分钟十位
        LED8[1] = stopwatchMinute % 10; // 显示分钟个位
        LED8[2] = 16;
        LED8[3] = stopwatchSecond / 10; // 显示秒十位
        LED8[4] = stopwatchSecond % 10; // 显示秒个位
        LED8[5] = 16;
        LED8[6] = stopwatchMSecond / 100;
        LED8[7] = (stopwatchMSecond % 100) / 10;

        Display(0xFF);
        break;
    case STOPWATCH_PAUSE:
        Display(0xFF);
    default:
        break;
    }
}

void Init()
{
    mode = SHOW;
    hourlyChime = 1;
    Chime = 0;

    EA = 1;      // 开启总中断
    IT0 = 1;     // 设置外部中断0为边沿触发方式
    EX0 = 1;     // 允许外部中断0
    IT1 = 1;     // 设置外部中断1为边沿触发方式
    EX1 = 1;     // 允许外部中断1
    PT0 = 1;     // 计时器0中断优先级为最高
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
