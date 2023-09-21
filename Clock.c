#include <REG51.H>
#include "LCD12864.h"

#include <string.h>

#define INTERVAL 10 // 间隔10毫秒

#define INIT_HOUR 23   // 初始化小时
#define INIT_MINUTE 59 // 初始化分钟
#define INIT_SECOND 58 // 初始化秒

#define INIT_YEAR 2020 // 初始化年
#define INIT_MONTH 2   // 初始化月
#define INIT_DAY 28    // 初始化日

#define INIT_ALARM 0         // 初始化闹钟功能，1表示响，0表示不响
#define INIT_ALARM_HOUR 23   // 初始化闹钟小时
#define INIT_ALARM_MINUTE 59 // 初始化闹钟分钟
#define INIT_ALARM_WEEKDAY 0 // 初始化闹钟星期
#define ALARMCLOCKTIMES 6    // 闹钟响铃次数

#define INIT_HOURLY_CHIME 0 // 初始化整点报时功能，1表示开，0表示关
#define HOURLYCHIMETIMES 3  // 整点响铃次数

unsigned char hour = INIT_HOUR;     // 初始化小时
unsigned char minute = INIT_MINUTE; // 初始化分钟
unsigned char second = INIT_SECOND; // 初始化秒

unsigned int year = INIT_YEAR;    // 初始化年
unsigned char month = INIT_MONTH; // 初始化月
unsigned char day = INIT_DAY;     // 初始化日

unsigned char weekday = 0; // 星期，星期初始化任务由init()函数负责

unsigned char alarmHour = INIT_ALARM_HOUR;       // 闹钟时
unsigned char alarmMinute = INIT_ALARM_MINUTE;   // 闹钟分
unsigned char alarmWeekday = INIT_ALARM_WEEKDAY; // 闹钟星期功能，低0位为1表示周日响，低1位为1表示周一响，以此类推

bit alarm = INIT_ALARM; // 闹钟功能，1表示响，0表示不响

bit hourlyChime = INIT_HOURLY_CHIME; // 整点报时功能，1表示开，0表示关

bit buttonDown = 0;            // 用于判断是否有按键按下，1为有，0为无
bit button = 0;                // 用于判断按下的是哪个按键
unsigned int checkCount = 0;   // 用于检查按键是长按还是短按，每按一毫秒该变量加一
unsigned char shortOrLang = 0; // 用于表示按键是长按还是短按的标志，0表示无效，1表示短按，2表示长按

unsigned char line1[16] = "                ";
unsigned char line2[16] = "                ";
unsigned char line3[16] = "                ";
unsigned char line4[16] = "                ";

unsigned char hourlyChimeTimes = 0; // 记录整点报时的响铃次数
unsigned char alarmClockTimes = 0;  // 记录闹钟的响铃次数

unsigned int stopwatchMSecond = 0; // 秒表的毫秒数
unsigned char stopwatchSecond = 0; // 秒表的秒数
unsigned char stopwatchMinute = 0; // 秒表的分钟数

unsigned char setAlarmHour = INIT_ALARM_HOUR;       // 设置闹钟时的变量
unsigned char setAlarmMinute = INIT_ALARM_MINUTE;   // 设置闹钟分的变量
unsigned char setAlarmWeekday = INIT_ALARM_WEEKDAY; // 设置闹钟星期的变量，低0位为1表示周日响，低1位为1表示周一响，以此类推

enum MODE
{
    SHOW,                 // 显示模式
    SET_YEAR,             // 设置年模式
    SET_MONTH,            // 设置月模式
    SET_DAY,              // 设置日模式
    SET_HOUR,             // 设置时模式
    SET_MINUTE,           // 设置分模式
    SET_SECOND,           // 设置秒模式
    STOPWATCH,            // 秒表模式
    STOPWATCH_START,      // 秒表计时模式
    STOPWATCH_PAUSE,      // 秒表暂停模式
    ALARMCLOCK,           // 闹钟模式
    ALARMCLOCK_HOUR,      // 设置闹钟时模式
    ALARMCLOCK_MINUTE,    // 设置闹钟分模式
    ALARMCLOCK_SUNDAY,    // 设置闹钟周日模式
    ALARMCLOCK_MONDAY,    // 设置闹钟周一模式
    ALARMCLOCK_TUESDAY,   // 设置闹钟周二模式
    ALARMCLOCK_WEDNESDAY, // 设置闹钟周三模式
    ALARMCLOCK_THURSDAY,  // 设置闹钟周四模式
    ALARMCLOCK_FRIDAY,    // 设置闹钟周五模式
    ALARMCLOCK_SATURDAY   // 设置闹钟周六模式
};

unsigned char mode = SHOW; // 模式

unsigned int interruptCount = 0; // 中断次数

sbit Chime = P2 ^ 3; // 蜂鸣器

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

// 初始化函数
void Init();
// 检查当前按键是长按还是短按
void Check();
// 按键短按处理程序
void ShortPress();
// 按键长按处理程序
void LongPress();
// 秒数增加处理程序
void SecondIncrease();
// 日期增加处理程序
void DateIncrease();
// 更新日期
void UpdateWeekday();
// 检查日期程序，返回0表示日期无误，返回1表示日期有误
bit CheckDate(unsigned int year, unsigned char month, unsigned char day);

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

// 初始化函数
void Init()
{

    UpdateWeekday(); // 初始化星期

    LCD12864_Init(); // 初始化液晶屏

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

// 检查当前按键是长按还是短按
void Check()
{
    checkCount++; // 按键时间计数，每过一毫秒加一。

    if (checkCount < 10 / INTERVAL) // 10ms延迟去抖动
        return;

    if (checkCount > 1010 / INTERVAL) // 如果按下时间大于一秒，为长按。
    {
        shortOrLang = 2;
        checkCount = 0;
        buttonDown = 0;
    }
    else // 如果按下时间小于一秒，则判断是否松开，若松开，则为短按，否则继续计数。
    {
        if ((button == 0 && P3_2 == 1) || (button == 1 && P3_3 == 1))
        {
            shortOrLang = 1;
            checkCount = 0;
            buttonDown = 0;
        }
    }
}

// 按键短按处理程序
void ShortPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SHOW:
            mode = STOPWATCH;
            break;
        case STOPWATCH:
            mode = ALARMCLOCK;
            break;
        case ALARMCLOCK:
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
        case SET_YEAR:
            mode = SET_MONTH;
            break;
        case SET_MONTH:
            mode = SET_DAY;
            break;
        case SET_DAY:
            mode = SET_YEAR;
            break;
        case ALARMCLOCK_HOUR:
            mode = ALARMCLOCK_MINUTE;
            break;
        case ALARMCLOCK_MINUTE:
            mode = ALARMCLOCK_SUNDAY;
            break;
        case ALARMCLOCK_SUNDAY:
            mode = ALARMCLOCK_MONDAY;
            break;
        case ALARMCLOCK_MONDAY:
            mode = ALARMCLOCK_TUESDAY;
            break;
        case ALARMCLOCK_TUESDAY:
            mode = ALARMCLOCK_WEDNESDAY;
            break;
        case ALARMCLOCK_WEDNESDAY:
            mode = ALARMCLOCK_THURSDAY;
            break;
        case ALARMCLOCK_THURSDAY:
            mode = ALARMCLOCK_FRIDAY;
            break;
        case ALARMCLOCK_FRIDAY:
            mode = ALARMCLOCK_SATURDAY;
            break;
        case ALARMCLOCK_SATURDAY:
            mode = ALARMCLOCK_HOUR;
            break;
        default:
            break;
        }
    }
    else
    {
        unsigned char setHour;
        unsigned char setMinute;
        unsigned char setSecond;
        switch (mode)
        {
        case SET_YEAR:
            year++;
            year %= 10000;

            UpdateWeekday();
            break;
        case SET_MONTH:
            month = (month % 12) + 1;

            UpdateWeekday();
            break;
        case SET_DAY:
            if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2) // 闰年2月
                day = (day % 29) + 1;
            else
                day = (day % Seg_Date[month - 1]) + 1;

            UpdateWeekday();
            break;
        case SET_HOUR:
            setHour++;
            setHour %= 24;

            break;
        case SET_MINUTE:
            setMinute++;
            setMinute %= 60;

            break;
        case SET_SECOND:
            setSecond++;
            setSecond %= 60;

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
        case ALARMCLOCK:
            alarm = !alarm;
            break;
        case ALARMCLOCK_HOUR:
            setAlarmHour++;
            setAlarmHour %= 24;

            break;
        case ALARMCLOCK_MINUTE:
            setAlarmMinute++;
            setAlarmMinute %= 60;

            break;
        case ALARMCLOCK_SUNDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 0); // 低0位取反

            break;
        case ALARMCLOCK_MONDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 1); // 低1位取反

            break;
        case ALARMCLOCK_TUESDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 2); // 低2位取反

            break;
        case ALARMCLOCK_WEDNESDAY:;
            setAlarmWeekday = setAlarmWeekday ^ (1 << 3); // 低3位取反

            break;
        case ALARMCLOCK_THURSDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 4); // 低4位取反

            break;
        case ALARMCLOCK_FRIDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 5); // 低5位取反

            break;
        case ALARMCLOCK_SATURDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 6); // 低6位取反

            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // 重置按键标志
}

// 按键长按处理程序
void LongPress()
{
    if (button == 0) // 如果按的是键A
    {
        switch (mode)
        {
        case SHOW:
            mode = SET_HOUR;
            break;
        case SET_YEAR:
        case SET_MONTH:
        case SET_DAY:
            if (!CheckDate(year, month, day)) // 如果日期无误才能确定
            {
                mode = SHOW;
            }
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:

            mode = SHOW;
            break;
        case ALARMCLOCK:
            mode = ALARMCLOCK_HOUR;
            break;
        case ALARMCLOCK_HOUR:
        case ALARMCLOCK_MINUTE:
        case ALARMCLOCK_SUNDAY:
        case ALARMCLOCK_MONDAY:
        case ALARMCLOCK_TUESDAY:
        case ALARMCLOCK_WEDNESDAY:
        case ALARMCLOCK_THURSDAY:
        case ALARMCLOCK_FRIDAY:
        case ALARMCLOCK_SATURDAY:
            alarmHour = setAlarmHour;
            alarmMinute = setAlarmMinute;
            alarmWeekday = setAlarmWeekday;
            mode = ALARMCLOCK;
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
        case SET_YEAR:
            year--;
            year %= 10000;

            UpdateWeekday();
            break;
        case SET_MONTH:
            month += 10;
            month = (month % 12) + 1;

            UpdateWeekday();
            break;
        case SET_DAY:
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

            UpdateWeekday();
            break;
        case STOPWATCH_PAUSE:
            mode = STOPWATCH;
            break;
        case ALARMCLOCK_HOUR:
        case ALARMCLOCK_MINUTE:
        case ALARMCLOCK_SUNDAY:
        case ALARMCLOCK_MONDAY:
        case ALARMCLOCK_TUESDAY:
        case ALARMCLOCK_WEDNESDAY:
        case ALARMCLOCK_THURSDAY:
        case ALARMCLOCK_FRIDAY:
        case ALARMCLOCK_SATURDAY:
            setAlarmHour = alarmHour;
            setAlarmMinute = alarmMinute;
            setAlarmWeekday = alarmWeekday;
            mode = ALARMCLOCK;
            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // 重置按键标志
}

// 秒数增加处理程序
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

// 日期增加处理程序
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
    weekday = ((month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1) % 7;
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

// 定时器0中断服务函数
void Timer0() interrupt 1
{
    unsigned char *p;
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1秒
        interruptCount = 0;
        SecondIncrease();
        if (hour == 0 && minute == 0 && second == 0)
        {
            DateIncrease();
            UpdateWeekday();
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

    if (mode == SHOW && alarm == 1)
    {
        if (hour == alarmHour && minute == alarmMinute && second == 00 && (alarmWeekday & (1 << weekday)) > 0 && interruptCount == 0)
            alarmClockTimes = 2 * ALARMCLOCKTIMES;
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

    if (mode == SHOW && hourlyChime == 1)
    {
        if (hourlyChimeTimes == 0 && minute == 0 && second == 0 && interruptCount == 0)
            hourlyChimeTimes = 2 * (HOURLYCHIMETIMES + 1); // 我也不知道这里为什么要加一，但事实就是它会响HOURLYCHIMETIMES-1次，所以要加一补上。
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
    case SHOW:
        ClearChar(line1);
        ClearChar(line2);
        ClearChar(line3);
        ClearChar(line4);
        line1[4] = hour / 10 + '0'; // 小时十位
        line1[5] = hour % 10 + '0'; // 小时个位
        line1[6] = ':';
        line1[7] = minute / 10 + '0'; // 分钟十位
        line1[8] = minute % 10 + '0'; // 分钟个位
        line1[9] = ':';
        line1[10] = second / 10 + '0'; // 秒钟十位
        line1[11] = second % 10 + '0'; // 秒钟个位

        line2[3] = year / 1000 + '0';     // 年份千位
        line2[4] = year / 100 % 10 + '0'; // 年份百位
        line2[5] = year / 10 % 10 + '0';  // 年份十位
        line2[6] = year % 10 + '0';       // 年份个位
        line2[7] = '-';
        line2[8] = month / 10 + '0'; // 月份十位
        line2[9] = month % 10 + '0'; // 月份个位
        line2[10] = '-';
        line2[11] = day / 10 + '0'; // 日期十位
        line2[12] = day % 10 + '0'; // 日期个位

        switch (weekday)
        {
        case 0:
            strncpy(line3 + 6, "周日", 4);
            break;
        case 1:
            strncpy(line3 + 6, "周一", 4);
            break;
        case 2:
            strncpy(line3 + 6, "周二", 4);
            break;
        case 3:
            strncpy(line3 + 6, "周三", 4);
            break;
        case 4:
            strncpy(line3 + 6, "周四", 4);
            break;
        case 5:
            strncpy(line3 + 6, "周五", 4);
            break;
        case 6:
            strncpy(line3 + 6, "周六", 4);
            break;
        default:
            break;
        }
        strncpy(line4 + 2, "整点报时  ", 10);
        if (hourlyChime)
            strncpy(line4 + 12, "开", 2);
        else
            strncpy(line4 + 12, "关", 2);

        LCD12864_DisplayOneLine(LINE1, line1, 16);
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        LCD12864_DisplayOneLine(LINE4, line4, 16);
        break;
    case SET_HOUR:

        break;
    case SET_MINUTE:

        break;
    case SET_SECOND:

        break;
    case SET_YEAR:

        break;
    case SET_MONTH:

        break;
    case SET_DAY:

        break;
    case STOPWATCH:
        stopwatchMSecond = 0;
        stopwatchSecond = 0;
        stopwatchMinute = 0;

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

        break;
    case STOPWATCH_PAUSE:

        break;
    case ALARMCLOCK:

        break;
    case ALARMCLOCK_HOUR:

        break;
    case ALARMCLOCK_MINUTE:

        break;
    case ALARMCLOCK_SUNDAY:

        break;
    case ALARMCLOCK_MONDAY:

        break;
    case ALARMCLOCK_TUESDAY:

        break;
    case ALARMCLOCK_WEDNESDAY:

        break;
    case ALARMCLOCK_THURSDAY:

        break;
    case ALARMCLOCK_FRIDAY:

        break;
    case ALARMCLOCK_SATURDAY:

        break;
    default:
        break;
    }
}
