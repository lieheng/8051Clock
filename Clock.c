#include <REG51.H>

#include "LCD12864.h"
#include "AT24C02.h"

#include <string.h>

#define INTERVAL 10 // 间隔10毫秒

#define INIT_HOUR 0    // 初始化小时
#define INIT_MINUTE 0  // 初始化分钟
#define INIT_SECOND 58 // 初始化秒

#define INIT_YEAR 2023 // 初始化年
#define INIT_MONTH 9   // 初始化月
#define INIT_DAY 19    // 初始化日

#define INIT_ALARM 1            // 初始化闹钟功能，1表示响，0表示不响
#define INIT_ALARM_HOUR 0       // 初始化闹钟小时
#define INIT_ALARM_MINUTE 1     // 初始化闹钟分钟
#define INIT_ALARM_WEEKDAY 0x7B // 初始化闹钟星期
#define ALARMCLOCKTIMES 6       // 闹钟响铃次数

#define INIT_HOURLY_CHIME 0 // 初始化整点报时功能，1表示开，0表示关
#define HOURLYCHIMETIMES 3  // 整点响铃次数

#define HOUR_ADDR 0x00   // 小时在AT24C02中的存储地址
#define MINUTE_ADDR 0x01 // 分钟在AT24C02中的存储地址
#define SECOND_ADDR 0x02 // 秒在AT24C02中的存储地址

#define YEAR_ADDR_H 0x03 // 年的高位在AT24C02中的存储地址
#define YEAR_ADDR_L 0x04 // 年的低位在AT24C02中的存储地址
#define MONTH_ADDR 0x05  // 月在AT24C02中的存储地址
#define DAY_ADDR 0x06    // 日在AT24C02中的存储地址

#define ALARM_HOUR_ADDR 0x07    // 闹钟时在AT24C02中的存储地址
#define ALARM_MINUTE_ADDR 0x08  // 闹钟分在AT24C02中的存储地址
#define ALARM_WEEKDAY_ADDR 0x09 // 闹钟星期在AT24C02中的存储地址

#define ALARM_ADDR 0x0A // 闹钟功能在AT24C02中的存储地址

#define HOURLY_CHIME_ADDR 0x0B // 整点报时功能在AT24C02中的存储地址

unsigned char xdata hour = INIT_HOUR;     // 初始化小时
unsigned char xdata minute = INIT_MINUTE; // 初始化分钟
unsigned char xdata second = INIT_SECOND; // 初始化秒

unsigned int xdata year = INIT_YEAR;    // 初始化年
unsigned char xdata month = INIT_MONTH; // 初始化月
unsigned char xdata day = INIT_DAY;     // 初始化日

unsigned char xdata weekday = 0; // 星期，星期初始化任务由init()函数负责

unsigned char alarmHour = INIT_ALARM_HOUR;       // 闹钟时
unsigned char alarmMinute = INIT_ALARM_MINUTE;   // 闹钟分
unsigned char alarmWeekday = INIT_ALARM_WEEKDAY; // 闹钟星期功能，低0位为1表示周日响，低1位为1表示周一响，以此类推

bit alarm = INIT_ALARM; // 闹钟功能，1表示响，0表示不响

bit hourlyChime = INIT_HOURLY_CHIME; // 整点报时功能，1表示开，0表示关

bit buttonDown = 0;            // 用于判断是否有按键按下，1为有，0为无
bit button = 0;                // 用于判断按下的是哪个按键
unsigned int checkCount = 0;   // 用于检查按键是长按还是短按
unsigned char shortOrLang = 0; // 用于表示按键是长按还是短按的标志，0表示无效，1表示短按，2表示长按

unsigned char line1[17] = "                "; // 第一行显示字符
unsigned char line2[17] = "                "; // 第二行显示字符
unsigned char line3[17] = "                "; // 第三行显示字符
unsigned char line4[17] = "                "; // 第四行显示字符

unsigned char hourlyChimeTimes = 0; // 记录整点报时的响铃次数
unsigned char alarmClockTimes = 0;  // 记录闹钟的响铃次数

unsigned int stopwatchMSecond = 0; // 秒表的毫秒数
unsigned char stopwatchSecond = 0; // 秒表的秒数
unsigned char stopwatchMinute = 0; // 秒表的分钟数
unsigned int recordNum = 0;        // 记录次数

unsigned char xdata setAlarmHour = INIT_ALARM_HOUR;       // 设置闹钟时的变量
unsigned char xdata setAlarmMinute = INIT_ALARM_MINUTE;   // 设置闹钟分的变量
unsigned char xdata setAlarmWeekday = INIT_ALARM_WEEKDAY; // 设置闹钟星期的变量，低0位为1表示周日响，低1位为1表示周一响，以此类推

unsigned char xdata setHour = INIT_HOUR;     // 设置小时的变量
unsigned char xdata setMinute = INIT_MINUTE; // 设置分钟的变量
unsigned char xdata setSecond = INIT_SECOND; // 设置秒的变量

enum MODE
{
    SHOW,                     // 显示模式
    SET_YEAR,                 // 设置年模式
    SET_MONTH,                // 设置月模式
    SET_DAY,                  // 设置日模式
    SET_HOUR,                 // 设置时模式
    SET_MINUTE,               // 设置分模式
    SET_SECOND,               // 设置秒模式
    STOPWATCH,                // 秒表模式
    STOPWATCH_START,          // 秒表计时模式
    STOPWATCH_PAUSE,          // 秒表暂停模式
    ALARMCLOCK,               // 闹钟模式
    SET_ALARMCLOCK_HOUR,      // 设置闹钟时模式
    SET_ALARMCLOCK_MINUTE,    // 设置闹钟分模式
    SET_ALARMCLOCK_SUNDAY,    // 设置闹钟周日模式
    SET_ALARMCLOCK_MONDAY,    // 设置闹钟周一模式
    SET_ALARMCLOCK_TUESDAY,   // 设置闹钟周二模式
    SET_ALARMCLOCK_WEDNESDAY, // 设置闹钟周三模式
    SET_ALARMCLOCK_THURSDAY,  // 设置闹钟周四模式
    SET_ALARMCLOCK_FRIDAY,    // 设置闹钟周五模式
    SET_ALARMCLOCK_SATURDAY   // 设置闹钟周六模式
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

// 初始化函数
void Init();
// 切换模式
void ChangeMode(MODE);
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
// 显示整点报时功能
void DisplayHourlyChime();
// 清空字符串
void ClearChar(unsigned char *str);
// 检查日期程序，返回0表示日期无误，返回1表示日期有误
bit CheckDate(unsigned int year, unsigned char month, unsigned char day);

sbit DQ = P0 ^ 0; // 温度传感器引脚

unsigned char code Array_Point[] = {0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9}; // 小数查表

void Delay_us(unsigned int uiUs); // us延时函数,12MHZ晶振有效

void DS18B20_Init(void);                      // DS18B20初始化
unsigned char DS18B20_ReadOneChar(void);      // 读取一个数据
void DS18B20_WriteOneChar(unsigned char dat); // 写入一个数据
unsigned int DS18B20_ReadTemperature(void);   // 读取温度

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
        DisplayTime();
        DisplayDate();
        DisplayWeekday();
        DisplayHourlyChime();
        break;
    case STOPWATCH:
        recordNum = 0;

        ClearChar(line1);
        ClearChar(line2);
        ClearChar(line3);
        ClearChar(line4);

        stopwatchMinute = 0;
        stopwatchSecond = 0;
        stopwatchMSecond = 0;

        line1[4] = stopwatchMinute / 10 + '0'; // 分钟十位
        line1[5] = stopwatchMinute % 10 + '0'; // 分钟个位
        line1[6] = ':';
        line1[7] = stopwatchSecond / 10 + '0'; // 秒十位
        line1[8] = stopwatchSecond % 10 + '0'; // 秒个位
        line1[9] = ':';
        line1[10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
        line1[11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

        LCD12864_DisplayOneLine(LINE1, line1, 16);
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        LCD12864_DisplayOneLine(LINE4, line4, 16);
        break;
    case ALARMCLOCK:
        ClearChar(line1);
        ClearChar(line2);
        ClearChar(line3);
        ClearChar(line4);

        setAlarmHour = alarmHour;
        setAlarmMinute = alarmMinute;
        setAlarmWeekday = alarmWeekday;

        line1[5] = alarmHour / 10 + '0';
        line1[6] = alarmHour % 10 + '0';
        line1[7] = ':';
        line1[8] = alarmMinute / 10 + '0';
        line1[9] = alarmMinute % 10 + '0';

        for (i = 0; i < 7; i++)
        {
            if (alarmWeekday & (1 << i))
                strncpy(line3 + 2 * i, "开", 2);
            else
                strncpy(line3 + 2 * i, "关", 2);
        }

        if (alarm)
            strncpy(line3 + 14, "开", 2);
        else
            strncpy(line3 + 14, "关", 2);

        strcpy(line4, "日一二");
        line4[6] = 0xC8;
        line4[7] = 0xFD;
        strcpy(line4 + 8, "四五六总");

        LCD12864_DisplayOneLine(LINE1, line1, 16);
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        LCD12864_DisplayOneLine(LINE4, line4, 16);
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
            ChangeMode(STOPWATCH);
            break;
        case STOPWATCH:
            ChangeMode(ALARMCLOCK);
            break;
        case STOPWATCH_START:
        case STOPWATCH_PAUSE:
            recordNum++;
            if (recordNum % 3 == 1)
            {
                ClearChar(line2);
                // 记录的编号
                line2[0] = (recordNum % 1000) / 100 + '0';
                line2[1] = (recordNum % 100) / 10 + '0';
                line2[2] = (recordNum % 10) + '0';
                // 记录的内容
                line2[4] = stopwatchMinute / 10 + '0'; // 分钟十位
                line2[5] = stopwatchMinute % 10 + '0'; // 分钟个位
                line2[6] = ':';
                line2[7] = stopwatchSecond / 10 + '0'; // 秒十位
                line2[8] = stopwatchSecond % 10 + '0'; // 秒个位
                line2[9] = ':';
                line2[10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
                line2[11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

                LCD12864_DisplayOneLine(LINE2, line2, 16);
            }
            else if (recordNum % 3 == 2)
            {
                ClearChar(line3);
                // 记录的编号
                line3[0] = (recordNum % 1000) / 100 + '0';
                line3[1] = (recordNum % 100) / 10 + '0';
                line3[2] = (recordNum % 10) + '0';
                // 记录的内容
                line3[4] = stopwatchMinute / 10 + '0'; // 分钟十位
                line3[5] = stopwatchMinute % 10 + '0'; // 分钟个位
                line3[6] = ':';
                line3[7] = stopwatchSecond / 10 + '0'; // 秒十位
                line3[8] = stopwatchSecond % 10 + '0'; // 秒个位
                line3[9] = ':';
                line3[10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
                line3[11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

                LCD12864_DisplayOneLine(LINE3, line3, 16);
            }
            else
            {
                ClearChar(line4);
                // 记录的编号
                line4[0] = (recordNum % 1000) / 100 + '0';
                line4[1] = (recordNum % 100) / 10 + '0';
                line4[2] = (recordNum % 10) + '0';
                // 记录的内容
                line4[4] = stopwatchMinute / 10 + '0'; // 分钟十位
                line4[5] = stopwatchMinute % 10 + '0'; // 分钟个位
                line4[6] = ':';
                line4[7] = stopwatchSecond / 10 + '0'; // 秒十位
                line4[8] = stopwatchSecond % 10 + '0'; // 秒个位
                line4[9] = ':';
                line4[10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
                line4[11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

                LCD12864_DisplayOneLine(LINE4, line4, 16);
            }
            break;
        case ALARMCLOCK:
            ChangeMode(SHOW);
            break;
        case SET_HOUR:
            line1[4] = setHour / 10 + '0';
            line1[5] = setHour % 10 + '0';
            ChangeMode(SET_MINUTE);
            break;
        case SET_MINUTE:
            line1[7] = setMinute / 10 + '0';
            line1[8] = setMinute % 10 + '0';
            ChangeMode(SET_SECOND);
            break;
        case SET_SECOND:
            line1[10] = setSecond / 10 + '0';
            line1[11] = setSecond % 10 + '0';
            ChangeMode(SET_HOUR);
            break;
        case SET_YEAR:
            line2[3] = year / 1000 + '0';     // 年份千位
            line2[4] = year / 100 % 10 + '0'; // 年份百位
            line2[5] = year / 10 % 10 + '0';  // 年份十位
            line2[6] = year % 10 + '0';       // 年份个位
            ChangeMode(SET_MONTH);
            break;
        case SET_MONTH:
            line2[8] = month / 10 + '0'; // 月份十位
            line2[9] = month % 10 + '0'; // 月份个位
            ChangeMode(SET_DAY);
            break;
        case SET_DAY:
            line2[11] = day / 10 + '0'; // 日期十位
            line2[12] = day % 10 + '0'; // 日期个位
            ChangeMode(SET_YEAR);
            break;
        case SET_ALARMCLOCK_HOUR:
            line1[5] = setAlarmHour / 10 + '0';
            line1[6] = setAlarmHour % 10 + '0';
            ChangeMode(SET_ALARMCLOCK_MINUTE);
            break;
        case SET_ALARMCLOCK_MINUTE:
            line1[8] = setAlarmMinute / 10 + '0';
            line1[9] = setAlarmMinute % 10 + '0';
            ChangeMode(SET_ALARMCLOCK_SUNDAY);
            break;
        case SET_ALARMCLOCK_SUNDAY:
            if (setAlarmWeekday & (1 << 0))
                strncpy(line3 + 0, "开", 2);
            else
                strncpy(line3 + 0, "关", 2);
            ChangeMode(SET_ALARMCLOCK_MONDAY);
            break;
        case SET_ALARMCLOCK_MONDAY:
            if (setAlarmWeekday & (1 << 1))
                strncpy(line3 + 2, "开", 2);
            else
                strncpy(line3 + 2, "关", 2);
            ChangeMode(SET_ALARMCLOCK_TUESDAY);
            break;
        case SET_ALARMCLOCK_TUESDAY:
            if (setAlarmWeekday & (1 << 2))
                strncpy(line3 + 4, "开", 2);
            else
                strncpy(line3 + 4, "关", 2);
            ChangeMode(SET_ALARMCLOCK_WEDNESDAY);
            break;
        case SET_ALARMCLOCK_WEDNESDAY:
            if (setAlarmWeekday & (1 << 3))
                strncpy(line3 + 6, "开", 2);
            else
                strncpy(line3 + 6, "关", 2);
            ChangeMode(SET_ALARMCLOCK_THURSDAY);
            break;
        case SET_ALARMCLOCK_THURSDAY:
            if (setAlarmWeekday & (1 << 4))
                strncpy(line3 + 8, "开", 2);
            else
                strncpy(line3 + 8, "关", 2);
            ChangeMode(SET_ALARMCLOCK_FRIDAY);
            break;
        case SET_ALARMCLOCK_FRIDAY:
            if (setAlarmWeekday & (1 << 5))
                strncpy(line3 + 10, "开", 2);
            else
                strncpy(line3 + 10, "关", 2);
            ChangeMode(SET_ALARMCLOCK_SATURDAY);
            break;
        case SET_ALARMCLOCK_SATURDAY:
            if (setAlarmWeekday & (1 << 6))
                strncpy(line3 + 12, "开", 2);
            else
                strncpy(line3 + 12, "关", 2);
            ChangeMode(SET_ALARMCLOCK_HOUR);
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
        case SET_YEAR:
            year++;
            year %= 10000;
            UpdateWeekday();
            DisplayWeekday();
            break;
        case SET_MONTH:
            month = (month % 12) + 1;
            UpdateWeekday();
            DisplayWeekday();
            break;
        case SET_DAY:
            if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2) // 闰年2月
                day = (day % 29) + 1;
            else
                day = (day % Seg_Date[month - 1]) + 1;
            UpdateWeekday();
            DisplayWeekday();
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
            ChangeMode(STOPWATCH_START);
            break;
        case STOPWATCH_START:
            ChangeMode(STOPWATCH_PAUSE);
            break;
        case STOPWATCH_PAUSE:
            ChangeMode(STOPWATCH_START);
            break;
        case ALARMCLOCK:
            alarm = !alarm;
            SaveAlarm();
            if (alarm)
                strncpy(line3 + 14, "开", 2);
            else
                strncpy(line3 + 14, "关", 2);
            LCD12864_DisplayOneLine(LINE3, line3, 16);
            break;
        case SET_ALARMCLOCK_HOUR:
            setAlarmHour++;
            setAlarmHour %= 24;
            break;
        case SET_ALARMCLOCK_MINUTE:
            setAlarmMinute++;
            setAlarmMinute %= 60;
            break;
        case SET_ALARMCLOCK_SUNDAY:
            setAlarmWeekday = setAlarmWeekday ^ (1 << 0); // 低0位取反
            break;
        case SET_ALARMCLOCK_MONDAY:
            setAlarmWeekday = setAlarmWeekday ^ (1 << 1); // 低1位取反
            break;
        case SET_ALARMCLOCK_TUESDAY:
            setAlarmWeekday = setAlarmWeekday ^ (1 << 2); // 低2位取反
            break;
        case SET_ALARMCLOCK_WEDNESDAY:;
            setAlarmWeekday = setAlarmWeekday ^ (1 << 3); // 低3位取反
            break;
        case SET_ALARMCLOCK_THURSDAY:
            setAlarmWeekday = setAlarmWeekday ^ (1 << 4); // 低4位取反
            break;
        case SET_ALARMCLOCK_FRIDAY:
            setAlarmWeekday = setAlarmWeekday ^ (1 << 5); // 低5位取反
            break;
        case SET_ALARMCLOCK_SATURDAY:
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
            setHour = hour;
            setMinute = minute;
            setSecond = second;
            ChangeMode(SET_HOUR);
            break;
        case SET_YEAR:
        case SET_MONTH:
        case SET_DAY:
            if (!CheckDate(year, month, day)) // 如果日期无误才能确定
            {
                SaveDate();
                ChangeMode(SHOW);
            }
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            hour = setHour;
            minute = setMinute;
            second = setSecond;
            SaveTime();
            ChangeMode(SHOW);
            break;
        case ALARMCLOCK:
            mode = SET_ALARMCLOCK_HOUR;
            break;
        case SET_ALARMCLOCK_HOUR:
        case SET_ALARMCLOCK_MINUTE:
        case SET_ALARMCLOCK_SUNDAY:
        case SET_ALARMCLOCK_MONDAY:
        case SET_ALARMCLOCK_TUESDAY:
        case SET_ALARMCLOCK_WEDNESDAY:
        case SET_ALARMCLOCK_THURSDAY:
        case SET_ALARMCLOCK_FRIDAY:
        case SET_ALARMCLOCK_SATURDAY:
            alarmHour = setAlarmHour;
            alarmMinute = setAlarmMinute;
            alarmWeekday = setAlarmWeekday;
            SaveAlarm();
            ChangeMode(ALARMCLOCK);
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
            ChangeMode(SET_YEAR);
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            ChangeMode(SHOW);
            break;
        case SET_YEAR:
            year--;
            year %= 10000;
            UpdateWeekday();
            DisplayWeekday();
            break;
        case SET_MONTH:
            month += 10;
            month = (month % 12) + 1;
            UpdateWeekday();
            DisplayWeekday();
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
            DisplayWeekday();
            break;
        case STOPWATCH_PAUSE:
            ChangeMode(STOPWATCH);
            break;
        case SET_ALARMCLOCK_HOUR:
        case SET_ALARMCLOCK_MINUTE:
        case SET_ALARMCLOCK_SUNDAY:
        case SET_ALARMCLOCK_MONDAY:
        case SET_ALARMCLOCK_TUESDAY:
        case SET_ALARMCLOCK_WEDNESDAY:
        case SET_ALARMCLOCK_THURSDAY:
        case SET_ALARMCLOCK_FRIDAY:
        case SET_ALARMCLOCK_SATURDAY:
            setAlarmHour = alarmHour;
            setAlarmMinute = alarmMinute;
            setAlarmWeekday = alarmWeekday;
            ChangeMode(ALARMCLOCK);
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

// 装载日期
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
    AT24C02_Write(ALARM_HOUR_ADDR, alarmHour);
    AT24C02_Write(ALARM_MINUTE_ADDR, alarmMinute);
    AT24C02_Write(ALARM_WEEKDAY_ADDR, alarmWeekday);
    AT24C02_Write(ALARM_ADDR, alarm);
}

// 装载闹钟
void LoadAlarm()
{
    alarmHour = AT24C02_Read(ALARM_HOUR_ADDR);
    alarmMinute = AT24C02_Read(ALARM_MINUTE_ADDR);
    alarmWeekday = AT24C02_Read(ALARM_WEEKDAY_ADDR);
    alarm = AT24C02_Read(ALARM_ADDR) ? 1 : 0;
}

// 显示时间
void DisplayTime()
{
    ClearChar(line1);

    line1[4] = hour / 10 + '0'; // 小时十位
    line1[5] = hour % 10 + '0'; // 小时个位
    line1[6] = ':';
    line1[7] = minute / 10 + '0'; // 分钟十位
    line1[8] = minute % 10 + '0'; // 分钟个位
    line1[9] = ':';
    line1[10] = second / 10 + '0'; // 秒钟十位
    line1[11] = second % 10 + '0'; // 秒钟个位

    LCD12864_DisplayOneLine(LINE1, line1, 16);
}

// 显示日期
void DisplayDate()
{
    ClearChar(line2);

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

    LCD12864_DisplayOneLine(LINE2, line2, 16);
}

// 显示星期
void DisplayWeekday()
{
    ClearChar(line3);

    switch (weekday)
    {
    case 0:
        strncpy(line3 + 2, "周日", 4);
        break;
    case 1:
        strncpy(line3 + 2, "周一", 4);
        break;
    case 2:
        strncpy(line3 + 2, "周二", 4);
        break;
    case 3:
        strncpy(line3 + 2, "周", 2);
        line3[4] = 0xC8;
        line3[5] = 0xFD;
        break;
    case 4:
        strncpy(line3 + 2, "周四", 4);
        break;
    case 5:
        strncpy(line3 + 2, "周五", 4);
        break;
    case 6:
        strncpy(line3 + 2, "周六", 4);
        break;
    default:
        break;
    }

    line3[8] = (DS18B20_ReadTemperature() >> 4) / 10 + '0';
    line3[9] = (DS18B20_ReadTemperature() >> 4) % 10 + '0';
    line3[10] = '.';
    line3[11] = Array_Point[DS18B20_ReadTemperature() & 0x000F] + '0';
    line3[12] = 0xA1;
    line3[13] = 0xE6;

    LCD12864_DisplayOneLine(LINE3, line3, 16);
}

// 显示整点报时功能
void DisplayHourlyChime()
{
    ClearChar(line4);
    strncpy(line4 + 2, "整点报时  ", 10);
    if (hourlyChime)
        strncpy(line4 + 12, "开", 2);
    else
        strncpy(line4 + 12, "关", 2);
    LCD12864_DisplayOneLine(LINE4, line4, 16);
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
        SaveTime();
        if (mode == SHOW)
            DisplayTime();
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

    if (mode == SHOW && hourlyChime == 1 && minute == 0 && second < 3)
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
    case SET_HOUR:
        if (interruptCount < (500 / INTERVAL))
        {
            line1[4] = setHour / 10 + '0';
            line1[5] = setHour % 10 + '0';
        }
        else
        {
            line1[4] = ' ';
            line1[5] = ' ';
        }
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        break;
    case SET_MINUTE:
        if (interruptCount < (500 / INTERVAL))
        {
            line1[7] = setMinute / 10 + '0';
            line1[8] = setMinute % 10 + '0';
        }
        else
        {
            line1[7] = ' ';
            line1[8] = ' ';
        }
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        break;
    case SET_SECOND:
        if (interruptCount < (500 / INTERVAL))
        {
            line1[10] = setSecond / 10 + '0';
            line1[11] = setSecond % 10 + '0';
        }
        else
        {
            line1[10] = ' ';
            line1[11] = ' ';
        }
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        break;
    case SET_YEAR:
        if (interruptCount < (500 / INTERVAL))
        {
            line2[3] = year / 1000 + '0';     // 年份千位
            line2[4] = year / 100 % 10 + '0'; // 年份百位
            line2[5] = year / 10 % 10 + '0';  // 年份十位
            line2[6] = year % 10 + '0';       // 年份个位
        }
        else
        {
            line2[3] = ' ';
            line2[4] = ' ';
            line2[5] = ' ';
            line2[6] = ' ';
        }
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        break;
    case SET_MONTH:
        if (interruptCount < (500 / INTERVAL))
        {
            line2[8] = month / 10 + '0'; // 月份十位
            line2[9] = month % 10 + '0'; // 月份个位
        }
        else
        {
            line2[8] = ' ';
            line2[9] = ' ';
        }
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        break;
    case SET_DAY:
        if (interruptCount < (500 / INTERVAL))
        {
            line2[11] = day / 10 + '0'; // 日期十位
            line2[12] = day % 10 + '0'; // 日期个位
        }
        else
        {
            line2[11] = ' ';
            line2[12] = ' ';
        }
        LCD12864_DisplayOneLine(LINE2, line2, 16);
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
        line1[4] = stopwatchMinute / 10 + '0'; // 分钟十位
        line1[5] = stopwatchMinute % 10 + '0'; // 分钟个位
        line1[6] = ':';
        line1[7] = stopwatchSecond / 10 + '0'; // 秒十位
        line1[8] = stopwatchSecond % 10 + '0'; // 秒个位
        line1[9] = ':';
        line1[10] = stopwatchMSecond / 100 + '0';     // 毫秒百位
        line1[11] = stopwatchMSecond / 10 % 10 + '0'; // 毫秒十位

        LCD12864_DisplayOneLine(LINE1, line1, 16);

        break;
    case SET_ALARMCLOCK_HOUR:
        if (interruptCount < (500 / INTERVAL))
        {
            line1[5] = setAlarmHour / 10 + '0';
            line1[6] = setAlarmHour % 10 + '0';
        }
        else
        {
            line1[5] = ' ';
            line1[6] = ' ';
        }
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        break;
    case SET_ALARMCLOCK_MINUTE:
        if (interruptCount < (500 / INTERVAL))
        {
            line1[8] = setAlarmMinute / 10 + '0';
            line1[9] = setAlarmMinute % 10 + '0';
        }
        else
        {
            line1[8] = ' ';
            line1[9] = ' ';
        }
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        break;
    case SET_ALARMCLOCK_SUNDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 0))
                strncpy(line3 + 0, "开", 2);
            else
                strncpy(line3 + 0, "关", 2);
        }
        else
        {
            line3[0] = ' ';
            line3[1] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_MONDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 1))
                strncpy(line3 + 2, "开", 2);
            else
                strncpy(line3 + 2, "关", 2);
        }
        else
        {
            line3[2] = ' ';
            line3[3] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_TUESDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 2))
                strncpy(line3 + 4, "开", 2);
            else
                strncpy(line3 + 4, "关", 2);
        }
        else
        {
            line3[4] = ' ';
            line3[5] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_WEDNESDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 3))
                strncpy(line3 + 6, "开", 2);
            else
                strncpy(line3 + 6, "关", 2);
        }
        else
        {
            line3[6] = ' ';
            line3[7] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_THURSDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 4))
                strncpy(line3 + 8, "开", 2);
            else
                strncpy(line3 + 8, "关", 2);
        }
        else
        {
            line3[8] = ' ';
            line3[9] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_FRIDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 5))
                strncpy(line3 + 10, "开", 2);
            else
                strncpy(line3 + 10, "关", 2);
        }
        else
        {
            line3[10] = ' ';
            line3[11] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    case SET_ALARMCLOCK_SATURDAY:
        if (interruptCount < (500 / INTERVAL))
        {
            if (setAlarmWeekday & (1 << 6))
                strncpy(line3 + 12, "开", 2);
            else
                strncpy(line3 + 12, "关", 2);
        }
        else
        {
            line3[12] = ' ';
            line3[13] = ' ';
        }
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        break;
    default:
        break;
    }
}

void DS18B20_Init(void)
{
    unsigned char x = 0;
    DQ = 1; // DQ复位
    Delay_us(10);
    // Delay(8);  //稍做延时,10us
    DQ = 0; // 单片机将DQ拉低
    Delay_us(500);
    // Delay(80); //精确延时 大于 480us ,498us
    DQ = 1; // 拉高总线
    Delay_us(154);
    // Delay(14);	//154us
    x = DQ; // 稍做延时后 如果x=0则初始化成功 x=1则初始化失败
    Delay_us(212);
    // Delay(20); //212us
}

unsigned char DS18B20_ReadOneChar(void)
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
        Delay_us(56);
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
        Delay_us(66);
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

void Delay_us(unsigned int uiUs) // us延时函数
{
    for (; uiUs > 0; uiUs--)
    {
        ;
    }
}
