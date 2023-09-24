#include <REG51.H>

#include "LCD12864.h"
#include "AT24C02.h"
#include "DS18B20.h"

#include <string.h>

#define INTERVAL 10 // 间隔10毫秒

#define INIT_HOUR 0    // 初始化时
#define INIT_MINUTE 0  // 初始化分
#define INIT_SECOND 58 // 初始化秒

#define INIT_YEAR 2023 // 初始化年
#define INIT_MONTH 9   // 初始化月
#define INIT_DAY 19    // 初始化日

#define INIT_ALARM_CLOCK 1            // 初始化闹钟功能，1表示开，0表示关
#define INIT_ALARM_CLOCK_HOUR 0       // 初始化闹钟时
#define INIT_ALARM_CLOCK_MINUTE 1     // 初始化闹钟分
#define INIT_ALARM_CLOCK_WEEKDAY 0x00 // 初始化闹钟星期
#define ALARM_CLOCK_TIMES 6           // 闹钟响铃次数

#define INIT_HOURLY_CHIME 0  // 初始化整点报时功能，1表示开，0表示关
#define HOURLY_CHIME_TIMES 3 // 整点响铃次数

#define HOUR_ADDR 0x00   // 时在AT24C02中的存储地址
#define MINUTE_ADDR 0x01 // 分在AT24C02中的存储地址
#define SECOND_ADDR 0x02 // 秒在AT24C02中的存储地址

#define YEAR_ADDR_H 0x03 // 年的高位在AT24C02中的存储地址
#define YEAR_ADDR_L 0x04 // 年的低位在AT24C02中的存储地址
#define MONTH_ADDR 0x05  // 月在AT24C02中的存储地址
#define DAY_ADDR 0x06    // 日在AT24C02中的存储地址

#define ALARM_CLOCK_HOUR_ADDR 0x07    // 闹钟时在AT24C02中的存储地址
#define ALARM_CLOCK_MINUTE_ADDR 0x08  // 闹钟分在AT24C02中的存储地址
#define ALARM_CLOCK_WEEKDAY_ADDR 0x09 // 闹钟星期在AT24C02中的存储地址

#define ALARM_CLOCK_ADDR 0x0A // 闹钟功能在AT24C02中的存储地址

#define HOURLY_CHIME_ADDR 0x0B // 整点报时功能在AT24C02中的存储地址

unsigned char xdata hour = INIT_HOUR;     // 初始化时
unsigned char xdata minute = INIT_MINUTE; // 初始化分
unsigned char xdata second = INIT_SECOND; // 初始化秒

unsigned int xdata year = INIT_YEAR;    // 初始化年
unsigned char xdata month = INIT_MONTH; // 初始化月
unsigned char xdata day = INIT_DAY;     // 初始化日

char xdata weekday = 0; // 星期，星期初始化任务由init()函数负责

unsigned char alarmClockHour = INIT_ALARM_CLOCK_HOUR;       // 初始化闹钟时
unsigned char alarmClockMinute = INIT_ALARM_CLOCK_MINUTE;   // 初始化闹钟分
unsigned char alarmClockWeekday = INIT_ALARM_CLOCK_WEEKDAY; // 初始化闹钟星期功能，低0位为1表示周日响，低1位为1表示周一响，以此类推

bit alarmClock = INIT_ALARM_CLOCK; // 初始化闹钟功能，1表示开，0表示关

bit hourlyChime = INIT_HOURLY_CHIME; // 初始化整点报时功能，1表示开，0表示关

bit buttonDown = 0;            // 用于判断是否有按键按下，1为有，0为无
bit button = 0;                // 用于判断按下的是哪个按键
unsigned int checkCount = 0;   // 用于检查按键是长按还是短按
unsigned char shortOrLang = 0; // 用于表示按键是长按还是短按的标志，0表示无效，1表示短按，2表示长按

unsigned char line[4][17] = {"                ", "                ", "                ", "                "}; // 各行的显示字符

unsigned char hourlyChimeTimes = 0; // 记录整点报时的响铃次数
unsigned char alarmClockTimes = 0;  // 记录闹钟的响铃次数

unsigned int stopwatchMSecond = 0; // 秒表的毫秒数
unsigned char stopwatchSecond = 0; // 秒表的秒数
unsigned char stopwatchMinute = 0; // 秒表的分数
unsigned int recordNum = 0;        // 记录次数

unsigned char xdata setAlarmClockHour = INIT_ALARM_CLOCK_HOUR;       // 设置闹钟时的变量
unsigned char xdata setAlarmClockMinute = INIT_ALARM_CLOCK_MINUTE;   // 设置闹钟分的变量
unsigned char xdata setAlarmClockWeekday = INIT_ALARM_CLOCK_WEEKDAY; // 设置闹钟星期的变量，低0位为1表示周日响，低1位为1表示周一响，以此类推

unsigned char xdata setHour = INIT_HOUR;     // 设置时的变量
unsigned char xdata setMinute = INIT_MINUTE; // 设置分的变量
unsigned char xdata setSecond = INIT_SECOND; // 设置秒的变量

unsigned char xdata setTimeIndex = 0;              // 用于判断当前设置的对象（时、分、秒）
unsigned char xdata setDateIndex = 0;              // 用于判断当前设置的对象（年、月、日）
bit setAlarmClockTimeIndex = 0;                    // 用于判断当前设置的对象（闹钟时、闹钟分）
unsigned char xdata setAlarmClockWeekdayIndex = 0; // 用于判断当前设置的对象（闹钟星期）

enum MODE
{
    SHOW,                    // 显示模式
    SET_TIME,                // 设置时间模式
    SET_DATE,                // 设置日期模式
    STOPWATCH,               // 秒表模式
    STOPWATCH_START,         // 秒表计时模式
    STOPWATCH_PAUSE,         // 秒表暂停模式
    ALARM_CLOCK,             // 闹钟模式
    SET_ALARM_CLOCK_TIME,    // 设置闹钟时间模式
    SET_ALARM_CLOCK_WEEKDAY, // 设置闹钟星期模式
};

unsigned char mode = SHOW; // 模式

unsigned int interruptCount = 0; // 中断次数

sbit Chime = P2 ^ 2; // 蜂鸣器

sbit P3_2 = P3 ^ 2; // 外部中断0的控制引脚
sbit P3_3 = P3 ^ 3; // 外部中断1的控制引脚

unsigned char code Seg_Date[] = {
    31, // 1月
    28, // 2月
    31, // 3月
    30, // 4月
    31, // 5月
    30, // 6月
    31, // 7月
    31, // 8月
    30, // 9月
    31, // 10月
    30, // 11月
    31  // 12月
};

// 初始化
void Init();
// 切换模式
void ChangeMode(MODE);
// 检查当前按键是长按还是短按
void Check();
// 按键短按
void ShortPress();
// 按键长按
void LongPress();
// 秒数增加
void SecondIncrease();
// 日期增加
void DateIncrease();
// 更新日期
void UpdateWeekday();
// 存储时间
void SaveTime();
// 装载时间
void LoadTime();
// 存储日期
void SaveDate();
// 装载日期
void LoadDate();
// 存储闹钟
void SaveAlarm();
// 装载闹钟
void LoadAlarm();
// 显示时间
void DisplayTime();
// 显示日期
void DisplayDate();
// 显示星期
void DisplayWeekday();
// 显示温度
void DisplayTemperature();
// 显示整点报时功能
void DisplayHourlyChime();
// 清空字符串
void ClearChar(unsigned char *str);
// 检查日期函数，返回0表示日期无误，返回1表示日期有误
bit CheckDate(unsigned int year, unsigned char month, unsigned char day);

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

// 初始化
void Init()
{
    LoadTime();
    LoadDate();
    LoadAlarm();

    LCD12864_Init(); // 初始化液晶屏

    UpdateWeekday(); // 初始化星期

    ChangeMode(SHOW);

    Chime = 0; // 初始化蜂鸣器引脚为0，防止损坏蜂鸣器

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

// 切换模式
void ChangeMode(MODE)
{
    unsigned char i = 0;
    mode = MODE;
    switch (mode)
    {
    case SHOW:
        ClearChar(line[0]);
        ClearChar(line[1]);
        ClearChar(line[2]);
        ClearChar(line[3]);

        DisplayTime();
        DisplayDate();
        DisplayWeekday();
        DisplayTemperature();
        DisplayHourlyChime();
        break;
    case STOPWATCH:
        recordNum = 0;

        ClearChar(line[0]);
        ClearChar(line[1]);
        ClearChar(line[2]);
        ClearChar(line[3]);

        stopwatchMinute = 0;
        stopwatchSecond = 0;
        stopwatchMSecond = 0;

        line[0][4] = stopwatchMinute / 10 + '0'; // 分十位
        line[0][5] = stopwatchMinute % 10 + '0'; // 分个位
        line[0][6] = ':';
        line[0][7] = stopwatchSecond / 10 + '0'; // 秒十位
        line[0][8] = stopwatchSecond % 10 + '0'; // 秒个位
        line[0][9] = ':';
        line[0][10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
        line[0][11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

        LCD12864_DisplayOneLine(0, line[0]);
        LCD12864_DisplayOneLine(1, line[1]);
        LCD12864_DisplayOneLine(2, line[2]);
        LCD12864_DisplayOneLine(3, line[3]);
        break;
    case ALARM_CLOCK:
        ClearChar(line[0]);
        ClearChar(line[1]);
        ClearChar(line[2]);
        ClearChar(line[3]);

        setAlarmClockHour = alarmClockHour;
        setAlarmClockMinute = alarmClockMinute;
        setAlarmClockWeekday = alarmClockWeekday;

        line[0][5] = alarmClockHour / 10 + '0';
        line[0][6] = alarmClockHour % 10 + '0';
        line[0][7] = ':';
        line[0][8] = alarmClockMinute / 10 + '0';
        line[0][9] = alarmClockMinute % 10 + '0';

        for (i = 0; i < 7; i++)
        {
            if (alarmClockWeekday & (1 << i))
                strncpy(line[2] + 2 * i, "开", 2);
            else
                strncpy(line[2] + 2 * i, "关", 2);
        }

        if (alarmClock)
            strncpy(line[2] + 14, "开", 2);
        else
            strncpy(line[2] + 14, "关", 2);

        strcpy(line[3], "日一二");
        line[3][6] = 0xC8;
        line[3][7] = 0xFD;
        strcpy(line[3] + 8, "四五六总");

        LCD12864_DisplayOneLine(0, line[0]);
        LCD12864_DisplayOneLine(1, line[1]);
        LCD12864_DisplayOneLine(2, line[2]);
        LCD12864_DisplayOneLine(3, line[3]);
    default:
        break;
    }
}

// 检查当前按键是长按还是短按
void Check()
{
    checkCount++; // 按键时间计数

    if (checkCount < 10 / INTERVAL) // 10ms延迟去抖动
        return;

    if (checkCount > 1010 / INTERVAL) // 如果按下时间大于一秒，为长按
    {
        shortOrLang = 2;
        checkCount = 0;
        buttonDown = 0;
    }
    else // 如果按下时间小于一秒，则判断是否松开，若松开，则为短按，否则继续计数
    {
        if ((button == 0 && P3_2 == 1) || (button == 1 && P3_3 == 1))
        {
            shortOrLang = 1;
            checkCount = 0;
            buttonDown = 0;
        }
    }
}

// 按键短按
void ShortPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SHOW:
            ChangeMode(STOPWATCH);
            break;
        case STOPWATCH:
            ChangeMode(ALARM_CLOCK);
            break;
        case STOPWATCH_START:
        case STOPWATCH_PAUSE:
            ClearChar(line[recordNum % 3 + 1]);

            line[recordNum % 3 + 1][0] = ((recordNum + 1) % 1000) / 100 + '0'; // 编号百位
            line[recordNum % 3 + 1][1] = ((recordNum + 1) % 100) / 10 + '0';   // 编号十位
            line[recordNum % 3 + 1][2] = ((recordNum + 1) % 10) + '0';         // 编号个位

            line[recordNum % 3 + 1][4] = stopwatchMinute / 10 + '0'; // 分十位
            line[recordNum % 3 + 1][5] = stopwatchMinute % 10 + '0'; // 分个位
            line[recordNum % 3 + 1][6] = ':';
            line[recordNum % 3 + 1][7] = stopwatchSecond / 10 + '0'; // 秒十位
            line[recordNum % 3 + 1][8] = stopwatchSecond % 10 + '0'; // 秒个位
            line[recordNum % 3 + 1][9] = ':';
            line[recordNum % 3 + 1][10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
            line[recordNum % 3 + 1][11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

            LCD12864_DisplayOneLine(recordNum % 3 + 1, line[recordNum % 3 + 1]);

            recordNum++;
            break;
        case ALARM_CLOCK:
            ChangeMode(SHOW);
            break;
        case SET_TIME:
            line[0][4] = setHour / 10 + '0';    // 时十位
            line[0][5] = setHour % 10 + '0';    // 时个位
            line[0][7] = setMinute / 10 + '0';  // 分十位
            line[0][8] = setMinute % 10 + '0';  // 分个位
            line[0][10] = setSecond / 10 + '0'; // 秒十位
            line[0][11] = setSecond % 10 + '0'; // 秒个位

            setTimeIndex = (setTimeIndex + 1) % 3;
            break;
        case SET_DATE:
            line[1][3] = year / 1000 + '0';     // 年千位
            line[1][4] = year / 100 % 10 + '0'; // 年百位
            line[1][5] = year / 10 % 10 + '0';  // 年十位
            line[1][6] = year % 10 + '0';       // 年个位
            line[1][8] = month / 10 + '0';      // 月十位
            line[1][9] = month % 10 + '0';      // 月个位
            line[1][11] = day / 10 + '0';       // 日十位
            line[1][12] = day % 10 + '0';       // 日个位

            setDateIndex = (setDateIndex + 1) % 3;
            break;
        case SET_ALARM_CLOCK_TIME:
            line[0][5] = setAlarmClockHour / 10 + '0';
            line[0][6] = setAlarmClockHour % 10 + '0';
            line[0][8] = setAlarmClockMinute / 10 + '0';
            line[0][9] = setAlarmClockMinute % 10 + '0';

            setAlarmClockTimeIndex = !setAlarmClockTimeIndex;

            if (!setAlarmClockTimeIndex)
                ChangeMode(SET_ALARM_CLOCK_WEEKDAY);

            break;
        case SET_ALARM_CLOCK_WEEKDAY:
            if (setAlarmClockWeekday & (1 << setAlarmClockWeekdayIndex))
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "开", 2);
            else
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "关", 2);

            setAlarmClockWeekdayIndex = (setAlarmClockWeekdayIndex + 1) % 7;

            if (setAlarmClockWeekdayIndex == 0)
                ChangeMode(SET_ALARM_CLOCK_TIME);

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
            SaveTime();
            DisplayHourlyChime();
            break;
        case SET_DATE:
            if (setDateIndex == 0)
            {
                year++;
                year %= 10000;
            }
            else if (setDateIndex == 1)
            {
                month = (month % 12) + 1;
            }
            else
            {
                if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2) // 闰年2月
                    day = (day % 29) + 1;
                else
                    day = (day % Seg_Date[month - 1]) + 1;
            }
            UpdateWeekday();
            DisplayWeekday();
            break;
        case SET_TIME:
            if (setTimeIndex == 0)
            {
                setHour++;
                setHour %= 24;
            }
            else if (setTimeIndex == 1)
            {
                setMinute++;
                setMinute %= 60;
            }
            else
            {
                setSecond++;
                setSecond %= 60;
            }
            break;
        case STOPWATCH:
            ChangeMode(STOPWATCH_START);
            break;
        case STOPWATCH_START:
            ChangeMode(STOPWATCH_PAUSE);
            break;
        case STOPWATCH_PAUSE:
            ChangeMode(STOPWATCH_START);
            break;
        case ALARM_CLOCK:
            alarmClock = !alarmClock;
            SaveAlarm();
            if (alarmClock)
                strncpy(line[2] + 14, "开", 2);
            else
                strncpy(line[2] + 14, "关", 2);
            LCD12864_DisplayOneLine(2, line[2]);
            break;
        case SET_ALARM_CLOCK_TIME:
            if (!setAlarmClockTimeIndex)
            {
                setAlarmClockHour++;
                setAlarmClockHour %= 24;
            }
            else
            {
                setAlarmClockMinute++;
                setAlarmClockMinute %= 60;
            }
            break;
        case SET_ALARM_CLOCK_WEEKDAY:
            setAlarmClockWeekday = setAlarmClockWeekday ^ (1 << setAlarmClockWeekdayIndex); // 对应位取反
            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // 重置按键标志
}

// 按键长按
void LongPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SHOW:
            setHour = hour;
            setMinute = minute;
            setSecond = second;
            setTimeIndex = 0;
            ChangeMode(SET_TIME);
            break;
        case SET_DATE:
            if (!CheckDate(year, month, day)) // 如果日期无误才能确定
            {
                SaveDate();
                ChangeMode(SHOW);
            }
            break;
        case SET_TIME:
            hour = setHour;
            minute = setMinute;
            second = setSecond;
            SaveTime();
            ChangeMode(SHOW);
            break;
        case ALARM_CLOCK:
            ChangeMode(SET_ALARM_CLOCK_TIME);
            break;
        case SET_ALARM_CLOCK_TIME:
        case SET_ALARM_CLOCK_WEEKDAY:
            alarmClockHour = setAlarmClockHour;
            alarmClockMinute = setAlarmClockMinute;
            alarmClockWeekday = setAlarmClockWeekday;
            SaveAlarm();
            ChangeMode(ALARM_CLOCK);
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
            setDateIndex = 0;
            ChangeMode(SET_DATE);
            break;
        case SET_TIME:
            ChangeMode(SHOW);
            break;
        case SET_DATE:
            if (setDateIndex == 0)
            {
                year--;
                year %= 10000;
            }
            else if (setDateIndex == 1)
            {
                month += 10;
                month = (month % 12) + 1;
            }
            else
            {
                if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2)
                {
                    day += 29 - 2;
                    day = (day % 29) + 1;
                }
                else
                {
                    day += Seg_Date[month - 1] - 2;
                    day = (day % Seg_Date[month - 1]) + 1;
                }
            }
            UpdateWeekday();
            DisplayWeekday();
            break;
        case STOPWATCH_PAUSE:
            ChangeMode(STOPWATCH);
            break;
        case SET_ALARM_CLOCK_TIME:
        case SET_ALARM_CLOCK_WEEKDAY:
            setAlarmClockHour = alarmClockHour;
            setAlarmClockMinute = alarmClockMinute;
            setAlarmClockWeekday = alarmClockWeekday;
            ChangeMode(ALARM_CLOCK);
            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // 重置按键标志
}

// 秒数增加
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

// 日期增加
void DateIncrease()
{
    day++;
    if (CheckDate(year, month, day))
    {
        day = 1;
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }
}

// 更新日期
void UpdateWeekday()
{
    weekday = (month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1;

    while (weekday < 0)
    {
        weekday += 7;
    }

    weekday %= 7;
}

// 存储时间
void SaveTime()
{
    AT24C02_Write(HOUR_ADDR, hour);
    AT24C02_Write(MINUTE_ADDR, minute);
    AT24C02_Write(SECOND_ADDR, second);
    AT24C02_Write(HOURLY_CHIME_ADDR, hourlyChime);
}

// 装载时间
void LoadTime()
{
    hour = AT24C02_Read(HOUR_ADDR);
    minute = AT24C02_Read(MINUTE_ADDR);
    second = AT24C02_Read(SECOND_ADDR);
    hourlyChime = AT24C02_Read(HOURLY_CHIME_ADDR) ? 1 : 0;
}

// 存储日期
void SaveDate()
{
    AT24C02_Write(YEAR_ADDR_L, year % 256);
    AT24C02_Write(YEAR_ADDR_H, year / 256);
    AT24C02_Write(MONTH_ADDR, month);
    AT24C02_Write(DAY_ADDR, day);
}

// 装载日期
void LoadDate()
{
    year = 0;
    year += AT24C02_Read(YEAR_ADDR_L);
    year += AT24C02_Read(YEAR_ADDR_H) * 256;
    month = AT24C02_Read(MONTH_ADDR);
    day = AT24C02_Read(DAY_ADDR);
}

// 存储闹钟
void SaveAlarm()
{
    AT24C02_Write(ALARM_CLOCK_HOUR_ADDR, alarmClockHour);
    AT24C02_Write(ALARM_CLOCK_MINUTE_ADDR, alarmClockMinute);
    AT24C02_Write(ALARM_CLOCK_WEEKDAY_ADDR, alarmClockWeekday);
    AT24C02_Write(ALARM_CLOCK_ADDR, alarmClock);
}

// 装载闹钟
void LoadAlarm()
{
    alarmClockHour = AT24C02_Read(ALARM_CLOCK_HOUR_ADDR);
    alarmClockMinute = AT24C02_Read(ALARM_CLOCK_MINUTE_ADDR);
    alarmClockWeekday = AT24C02_Read(ALARM_CLOCK_WEEKDAY_ADDR);
    alarmClock = AT24C02_Read(ALARM_CLOCK_ADDR) ? 1 : 0;
}

// 显示时间
void DisplayTime()
{
    line[0][4] = hour / 10 + '0'; // 时十位
    line[0][5] = hour % 10 + '0'; // 时个位
    line[0][6] = ':';
    line[0][7] = minute / 10 + '0'; // 分十位
    line[0][8] = minute % 10 + '0'; // 分个位
    line[0][9] = ':';
    line[0][10] = second / 10 + '0'; // 秒十位
    line[0][11] = second % 10 + '0'; // 秒个位

    LCD12864_DisplayOneLine(0, line[0]);
}

// 显示日期
void DisplayDate()
{
    line[1][3] = year / 1000 + '0';     // 年千位
    line[1][4] = year / 100 % 10 + '0'; // 年百位
    line[1][5] = year / 10 % 10 + '0';  // 年十位
    line[1][6] = year % 10 + '0';       // 年个位
    line[1][7] = '-';
    line[1][8] = month / 10 + '0'; // 月十位
    line[1][9] = month % 10 + '0'; // 月个位
    line[1][10] = '-';
    line[1][11] = day / 10 + '0'; // 日十位
    line[1][12] = day % 10 + '0'; // 日个位

    LCD12864_DisplayOneLine(1, line[1]);
}

// 显示星期
void DisplayWeekday()
{
    switch (weekday)
    {
    case 0:
        strncpy(line[2] + 2, "周日", 4);
        break;
    case 1:
        strncpy(line[2] + 2, "周一", 4);
        break;
    case 2:
        strncpy(line[2] + 2, "周二", 4);
        break;
    case 3:
        strncpy(line[2] + 2, "周", 2);
        line[2][4] = 0xC8;
        line[2][5] = 0xFD;
        break;
    case 4:
        strncpy(line[2] + 2, "周四", 4);
        break;
    case 5:
        strncpy(line[2] + 2, "周五", 4);
        break;
    case 6:
        strncpy(line[2] + 2, "周六", 4);
        break;
    default:
        break;
    }

    LCD12864_DisplayOneLine(2, line[2]);
}
// 显示温度
void DisplayTemperature()
{
    line[2][8] = (DS18B20_ReadTemperature() >> 4) / 10 + '0';
    line[2][9] = (DS18B20_ReadTemperature() >> 4) % 10 + '0';
    line[2][10] = '.';
    line[2][11] = Array_Point[DS18B20_ReadTemperature() & 0x000F] + '0';
    line[2][12] = 0xA1;
    line[2][13] = 0xE6;

    LCD12864_DisplayOneLine(2, line[2]);
}

// 显示整点报时功能
void DisplayHourlyChime()
{
    strncpy(line[3] + 2, "整点报时  ", 10);
    if (hourlyChime)
        strncpy(line[3] + 12, "开", 2);
    else
        strncpy(line[3] + 12, "关", 2);

    LCD12864_DisplayOneLine(3, line[3]);
}

// 检查日期程序，返回0表示日期无误，返回1表示日期有误
bit CheckDate(unsigned int year, unsigned char month, unsigned char day)
{
    if (month > 12 || month < 1)
        return 1; // 月份有误

    if (day < 1)
        return 1; // 日期有误

    if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2)
    {
        if (day > 29)
            return 1; // 闰年2月29日有误
    }
    else
    {
        if (day > Seg_Date[month - 1])
            return 1;
    }

    return 0;
}

// 清空字符串
void ClearChar(unsigned char *str)
{
    unsigned char len = strlen(str);
    unsigned char i = 0;
    for (i = 0; i < len; i++)
    {
        str[i] = ' ';
    }
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

// 计时器0中断服务函数
void Timer0() interrupt 1
{
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1秒
        interruptCount = 0;
        SecondIncrease();
        SaveTime();
        if (mode == SHOW)
        {
            DisplayTime();
            DisplayTemperature();
        }
        if (hour == 0 && minute == 0 && second == 0)
        {
            DateIncrease();
            UpdateWeekday();
            SaveDate();
            if (mode == SHOW)
            {
                DisplayDate();
                DisplayWeekday();
            }
        }
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

    if (mode == SHOW && alarmClock == 1)
    {
        if (hour == alarmClockHour && minute == alarmClockMinute && second == 00 && (alarmClockWeekday & (1 << weekday)) > 0 && interruptCount == 0)
            alarmClockTimes = 2 * ALARM_CLOCK_TIMES;
        if (alarmClockTimes != 0)
        {
            if ((0 < interruptCount && interruptCount < (250 / INTERVAL)) || ((500 / INTERVAL) < interruptCount && interruptCount < (750 / INTERVAL)))
                Chime = !Chime;
            else
                Chime = 0;
            if (interruptCount % (250 / INTERVAL) == 0)
            {
                alarmClockTimes--;
                Chime = 0;
            }
        }
    }

    if (mode == SHOW && hourlyChime == 1 && minute == 0 && second < 3)
    {
        if (hourlyChimeTimes == 0 && minute == 0 && second == 0 && interruptCount == 0)
            hourlyChimeTimes = 2 * (HOURLY_CHIME_TIMES + 1);
        if (hourlyChimeTimes != 0)
        {
            if (interruptCount > (500 / INTERVAL))
                Chime = !Chime;
            else
                Chime = 0;
            if (interruptCount % (500 / INTERVAL) == 0)
            {
                hourlyChimeTimes--;
                Chime = 0;
            }
        }
    }

    switch (mode)
    {
    case SET_TIME:
        if (setTimeIndex == 0)
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[0][4] = setHour / 10 + '0';
                line[0][5] = setHour % 10 + '0';
            }
            else
            {
                line[0][4] = ' ';
                line[0][5] = ' ';
            }
        }
        else if (setTimeIndex == 1)
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[0][7] = setMinute / 10 + '0';
                line[0][8] = setMinute % 10 + '0';
            }
            else
            {
                line[0][7] = ' ';
                line[0][8] = ' ';
            }
        }
        else
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[0][10] = setSecond / 10 + '0';
                line[0][11] = setSecond % 10 + '0';
            }
            else
            {
                line[0][10] = ' ';
                line[0][11] = ' ';
            }
        }
        LCD12864_DisplayOneLine(0, line[0]);
        break;
    case SET_DATE:
        if (setDateIndex == 0)
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[1][3] = year / 1000 + '0';
                line[1][4] = year / 100 % 10 + '0';
                line[1][5] = year / 10 % 10 + '0';
                line[1][6] = year % 10 + '0';
            }
            else
            {
                line[1][3] = ' ';
                line[1][4] = ' ';
                line[1][5] = ' ';
                line[1][6] = ' ';
            }
        }
        else if (setDateIndex == 1)
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[1][8] = month / 10 + '0';
                line[1][9] = month % 10 + '0';
            }
            else
            {
                line[1][8] = ' ';
                line[1][9] = ' ';
            }
        }
        else
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[1][11] = day / 10 + '0';
                line[1][12] = day % 10 + '0';
            }
            else
            {
                line[1][11] = ' ';
                line[1][12] = ' ';
            }
        }

        LCD12864_DisplayOneLine(1, line[1]);
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
        line[0][4] = stopwatchMinute / 10 + '0';
        line[0][5] = stopwatchMinute % 10 + '0';
        line[0][6] = ':';
        line[0][7] = stopwatchSecond / 10 + '0';
        line[0][8] = stopwatchSecond % 10 + '0';
        line[0][9] = ':';
        line[0][10] = stopwatchMSecond / 100 + '0';
        line[0][11] = stopwatchMSecond / 10 % 10 + '0';

        LCD12864_DisplayOneLine(0, line[0]);

        break;
    case SET_ALARM_CLOCK_TIME:
        if (!setAlarmClockTimeIndex)
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[0][5] = setAlarmClockHour / 10 + '0';
                line[0][6] = setAlarmClockHour % 10 + '0';
            }
            else
            {
                line[0][5] = ' ';
                line[0][6] = ' ';
            }
        }
        else
        {
            if (interruptCount < (500 / INTERVAL))
            {
                line[0][8] = setAlarmClockMinute / 10 + '0';
                line[0][9] = setAlarmClockMinute % 10 + '0';
            }
            else
            {
                line[0][8] = ' ';
                line[0][9] = ' ';
            }
        }
        LCD12864_DisplayOneLine(0, line[0]);
        break;
    case SET_ALARM_CLOCK_WEEKDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmClockWeekday & (1 << setAlarmClockWeekdayIndex))
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "开", 2);
            else
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "关", 2);
        }
        else
        {
            line[2][setAlarmClockWeekdayIndex * 2] = ' ';
            line[2][setAlarmClockWeekdayIndex * 2 + 1] = ' ';
        }
        LCD12864_DisplayOneLine(2, line[2]);
        break;
    default:
        break;
    }
}
