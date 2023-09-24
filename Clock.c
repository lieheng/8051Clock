#include <REG51.H>

#include "LCD12864.h"
#include "AT24C02.h"
#include "DS18B20.h"

#include <string.h>

#define INTERVAL 10 // ���10����

#define INIT_HOUR 0    // ��ʼ��ʱ
#define INIT_MINUTE 0  // ��ʼ����
#define INIT_SECOND 58 // ��ʼ����

#define INIT_YEAR 2023 // ��ʼ����
#define INIT_MONTH 9   // ��ʼ����
#define INIT_DAY 19    // ��ʼ����

#define INIT_ALARM_CLOCK 1            // ��ʼ�����ӹ��ܣ�1��ʾ����0��ʾ��
#define INIT_ALARM_CLOCK_HOUR 0       // ��ʼ������ʱ
#define INIT_ALARM_CLOCK_MINUTE 1     // ��ʼ�����ӷ�
#define INIT_ALARM_CLOCK_WEEKDAY 0x00 // ��ʼ����������
#define ALARM_CLOCK_TIMES 6           // �����������

#define INIT_HOURLY_CHIME 0  // ��ʼ�����㱨ʱ���ܣ�1��ʾ����0��ʾ��
#define HOURLY_CHIME_TIMES 3 // �����������

#define HOUR_ADDR 0x00   // ʱ��AT24C02�еĴ洢��ַ
#define MINUTE_ADDR 0x01 // ����AT24C02�еĴ洢��ַ
#define SECOND_ADDR 0x02 // ����AT24C02�еĴ洢��ַ

#define YEAR_ADDR_H 0x03 // ��ĸ�λ��AT24C02�еĴ洢��ַ
#define YEAR_ADDR_L 0x04 // ��ĵ�λ��AT24C02�еĴ洢��ַ
#define MONTH_ADDR 0x05  // ����AT24C02�еĴ洢��ַ
#define DAY_ADDR 0x06    // ����AT24C02�еĴ洢��ַ

#define ALARM_CLOCK_HOUR_ADDR 0x07    // ����ʱ��AT24C02�еĴ洢��ַ
#define ALARM_CLOCK_MINUTE_ADDR 0x08  // ���ӷ���AT24C02�еĴ洢��ַ
#define ALARM_CLOCK_WEEKDAY_ADDR 0x09 // ����������AT24C02�еĴ洢��ַ

#define ALARM_CLOCK_ADDR 0x0A // ���ӹ�����AT24C02�еĴ洢��ַ

#define HOURLY_CHIME_ADDR 0x0B // ���㱨ʱ������AT24C02�еĴ洢��ַ

unsigned char xdata hour = INIT_HOUR;     // ��ʼ��ʱ
unsigned char xdata minute = INIT_MINUTE; // ��ʼ����
unsigned char xdata second = INIT_SECOND; // ��ʼ����

unsigned int xdata year = INIT_YEAR;    // ��ʼ����
unsigned char xdata month = INIT_MONTH; // ��ʼ����
unsigned char xdata day = INIT_DAY;     // ��ʼ����

char xdata weekday = 0; // ���ڣ����ڳ�ʼ��������init()��������

unsigned char alarmClockHour = INIT_ALARM_CLOCK_HOUR;       // ��ʼ������ʱ
unsigned char alarmClockMinute = INIT_ALARM_CLOCK_MINUTE;   // ��ʼ�����ӷ�
unsigned char alarmClockWeekday = INIT_ALARM_CLOCK_WEEKDAY; // ��ʼ���������ڹ��ܣ���0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

bit alarmClock = INIT_ALARM_CLOCK; // ��ʼ�����ӹ��ܣ�1��ʾ����0��ʾ��

bit hourlyChime = INIT_HOURLY_CHIME; // ��ʼ�����㱨ʱ���ܣ�1��ʾ����0��ʾ��

bit buttonDown = 0;            // �����ж��Ƿ��а������£�1Ϊ�У�0Ϊ��
bit button = 0;                // �����жϰ��µ����ĸ�����
unsigned int checkCount = 0;   // ���ڼ�鰴���ǳ������Ƕ̰�
unsigned char shortOrLang = 0; // ���ڱ�ʾ�����ǳ������Ƕ̰��ı�־��0��ʾ��Ч��1��ʾ�̰���2��ʾ����

unsigned char line[4][17] = {"                ", "                ", "                ", "                "}; // ���е���ʾ�ַ�

unsigned char hourlyChimeTimes = 0; // ��¼���㱨ʱ���������
unsigned char alarmClockTimes = 0;  // ��¼���ӵ��������

unsigned int stopwatchMSecond = 0; // ���ĺ�����
unsigned char stopwatchSecond = 0; // ��������
unsigned char stopwatchMinute = 0; // ���ķ���
unsigned int recordNum = 0;        // ��¼����

unsigned char xdata setAlarmClockHour = INIT_ALARM_CLOCK_HOUR;       // ��������ʱ�ı���
unsigned char xdata setAlarmClockMinute = INIT_ALARM_CLOCK_MINUTE;   // �������ӷֵı���
unsigned char xdata setAlarmClockWeekday = INIT_ALARM_CLOCK_WEEKDAY; // �����������ڵı�������0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

unsigned char xdata setHour = INIT_HOUR;     // ����ʱ�ı���
unsigned char xdata setMinute = INIT_MINUTE; // ���÷ֵı���
unsigned char xdata setSecond = INIT_SECOND; // ������ı���

unsigned char xdata setTimeIndex = 0;              // �����жϵ�ǰ���õĶ���ʱ���֡��룩
unsigned char xdata setDateIndex = 0;              // �����жϵ�ǰ���õĶ����ꡢ�¡��գ�
bit setAlarmClockTimeIndex = 0;                    // �����жϵ�ǰ���õĶ�������ʱ�����ӷ֣�
unsigned char xdata setAlarmClockWeekdayIndex = 0; // �����жϵ�ǰ���õĶ����������ڣ�

enum MODE
{
    SHOW,                    // ��ʾģʽ
    SET_TIME,                // ����ʱ��ģʽ
    SET_DATE,                // ��������ģʽ
    STOPWATCH,               // ���ģʽ
    STOPWATCH_START,         // ����ʱģʽ
    STOPWATCH_PAUSE,         // �����ͣģʽ
    ALARM_CLOCK,             // ����ģʽ
    SET_ALARM_CLOCK_TIME,    // ��������ʱ��ģʽ
    SET_ALARM_CLOCK_WEEKDAY, // ������������ģʽ
};

unsigned char mode = SHOW; // ģʽ

unsigned int interruptCount = 0; // �жϴ���

sbit Chime = P2 ^ 2; // ������

sbit P3_2 = P3 ^ 2; // �ⲿ�ж�0�Ŀ�������
sbit P3_3 = P3 ^ 3; // �ⲿ�ж�1�Ŀ�������

unsigned char code Seg_Date[] = {
    31, // 1��
    28, // 2��
    31, // 3��
    30, // 4��
    31, // 5��
    30, // 6��
    31, // 7��
    31, // 8��
    30, // 9��
    31, // 10��
    30, // 11��
    31  // 12��
};

// ��ʼ��
void Init();
// �л�ģʽ
void ChangeMode(MODE);
// ��鵱ǰ�����ǳ������Ƕ̰�
void Check();
// �����̰�
void ShortPress();
// ��������
void LongPress();
// ��������
void SecondIncrease();
// ��������
void DateIncrease();
// ��������
void UpdateWeekday();
// �洢ʱ��
void SaveTime();
// װ��ʱ��
void LoadTime();
// �洢����
void SaveDate();
// װ������
void LoadDate();
// �洢����
void SaveAlarm();
// װ������
void LoadAlarm();
// ��ʾʱ��
void DisplayTime();
// ��ʾ����
void DisplayDate();
// ��ʾ����
void DisplayWeekday();
// ��ʾ�¶�
void DisplayTemperature();
// ��ʾ���㱨ʱ����
void DisplayHourlyChime();
// ����ַ���
void ClearChar(unsigned char *str);
// ������ں���������0��ʾ�������󣬷���1��ʾ��������
bit CheckDate(unsigned int year, unsigned char month, unsigned char day);

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

// ��ʼ��
void Init()
{
    LoadTime();
    LoadDate();
    LoadAlarm();

    LCD12864_Init(); // ��ʼ��Һ����

    UpdateWeekday(); // ��ʼ������

    ChangeMode(SHOW);

    Chime = 0; // ��ʼ������������Ϊ0����ֹ�𻵷�����

    EA = 1;      // �������ж�
    IT0 = 1;     // �����ⲿ�ж�0Ϊ���ش�����ʽ
    EX0 = 1;     // �����ⲿ�ж�0
    IT1 = 1;     // �����ⲿ�ж�1Ϊ���ش�����ʽ
    EX1 = 1;     // �����ⲿ�ж�1
    PT0 = 1;     // ��ʱ��0�ж����ȼ�Ϊ���
    TMOD = 0x01; // ���ü�ʱ��0�����ڷ�ʽ1
    ET0 = 1;     // �����ʱ��0�ж�
    TR0 = 1;     // ������ʱ��

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;
}

// �л�ģʽ
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

        line[0][4] = stopwatchMinute / 10 + '0'; // ��ʮλ
        line[0][5] = stopwatchMinute % 10 + '0'; // �ָ�λ
        line[0][6] = ':';
        line[0][7] = stopwatchSecond / 10 + '0'; // ��ʮλ
        line[0][8] = stopwatchSecond % 10 + '0'; // ���λ
        line[0][9] = ':';
        line[0][10] = stopwatchMSecond / 100 + '0';     // �����λ
        line[0][11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ

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
                strncpy(line[2] + 2 * i, "��", 2);
            else
                strncpy(line[2] + 2 * i, "��", 2);
        }

        if (alarmClock)
            strncpy(line[2] + 14, "��", 2);
        else
            strncpy(line[2] + 14, "��", 2);

        strcpy(line[3], "��һ��");
        line[3][6] = 0xC8;
        line[3][7] = 0xFD;
        strcpy(line[3] + 8, "��������");

        LCD12864_DisplayOneLine(0, line[0]);
        LCD12864_DisplayOneLine(1, line[1]);
        LCD12864_DisplayOneLine(2, line[2]);
        LCD12864_DisplayOneLine(3, line[3]);
    default:
        break;
    }
}

// ��鵱ǰ�����ǳ������Ƕ̰�
void Check()
{
    checkCount++; // ����ʱ�����

    if (checkCount < 10 / INTERVAL) // 10ms�ӳ�ȥ����
        return;

    if (checkCount > 1010 / INTERVAL) // �������ʱ�����һ�룬Ϊ����
    {
        shortOrLang = 2;
        checkCount = 0;
        buttonDown = 0;
    }
    else // �������ʱ��С��һ�룬���ж��Ƿ��ɿ������ɿ�����Ϊ�̰��������������
    {
        if ((button == 0 && P3_2 == 1) || (button == 1 && P3_3 == 1))
        {
            shortOrLang = 1;
            checkCount = 0;
            buttonDown = 0;
        }
    }
}

// �����̰�
void ShortPress()
{
    if (button == 0) // ��������Ǽ�A
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

            line[recordNum % 3 + 1][0] = ((recordNum + 1) % 1000) / 100 + '0'; // ��Ű�λ
            line[recordNum % 3 + 1][1] = ((recordNum + 1) % 100) / 10 + '0';   // ���ʮλ
            line[recordNum % 3 + 1][2] = ((recordNum + 1) % 10) + '0';         // ��Ÿ�λ

            line[recordNum % 3 + 1][4] = stopwatchMinute / 10 + '0'; // ��ʮλ
            line[recordNum % 3 + 1][5] = stopwatchMinute % 10 + '0'; // �ָ�λ
            line[recordNum % 3 + 1][6] = ':';
            line[recordNum % 3 + 1][7] = stopwatchSecond / 10 + '0'; // ��ʮλ
            line[recordNum % 3 + 1][8] = stopwatchSecond % 10 + '0'; // ���λ
            line[recordNum % 3 + 1][9] = ':';
            line[recordNum % 3 + 1][10] = stopwatchMSecond / 100 + '0';     // �����λ
            line[recordNum % 3 + 1][11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ

            LCD12864_DisplayOneLine(recordNum % 3 + 1, line[recordNum % 3 + 1]);

            recordNum++;
            break;
        case ALARM_CLOCK:
            ChangeMode(SHOW);
            break;
        case SET_TIME:
            line[0][4] = setHour / 10 + '0';    // ʱʮλ
            line[0][5] = setHour % 10 + '0';    // ʱ��λ
            line[0][7] = setMinute / 10 + '0';  // ��ʮλ
            line[0][8] = setMinute % 10 + '0';  // �ָ�λ
            line[0][10] = setSecond / 10 + '0'; // ��ʮλ
            line[0][11] = setSecond % 10 + '0'; // ���λ

            setTimeIndex = (setTimeIndex + 1) % 3;
            break;
        case SET_DATE:
            line[1][3] = year / 1000 + '0';     // ��ǧλ
            line[1][4] = year / 100 % 10 + '0'; // ���λ
            line[1][5] = year / 10 % 10 + '0';  // ��ʮλ
            line[1][6] = year % 10 + '0';       // ���λ
            line[1][8] = month / 10 + '0';      // ��ʮλ
            line[1][9] = month % 10 + '0';      // �¸�λ
            line[1][11] = day / 10 + '0';       // ��ʮλ
            line[1][12] = day % 10 + '0';       // �ո�λ

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
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "��", 2);
            else
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "��", 2);

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
                if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2) // ����2��
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
                strncpy(line[2] + 14, "��", 2);
            else
                strncpy(line[2] + 14, "��", 2);
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
            setAlarmClockWeekday = setAlarmClockWeekday ^ (1 << setAlarmClockWeekdayIndex); // ��Ӧλȡ��
            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // ���ð�����־
}

// ��������
void LongPress()
{
    if (button == 0) // ��������Ǽ�A
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
            if (!CheckDate(year, month, day)) // ��������������ȷ��
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

    shortOrLang = 0; // ���ð�����־
}

// ��������
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

// ��������
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

// ��������
void UpdateWeekday()
{
    weekday = (month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1;

    while (weekday < 0)
    {
        weekday += 7;
    }

    weekday %= 7;
}

// �洢ʱ��
void SaveTime()
{
    AT24C02_Write(HOUR_ADDR, hour);
    AT24C02_Write(MINUTE_ADDR, minute);
    AT24C02_Write(SECOND_ADDR, second);
    AT24C02_Write(HOURLY_CHIME_ADDR, hourlyChime);
}

// װ��ʱ��
void LoadTime()
{
    hour = AT24C02_Read(HOUR_ADDR);
    minute = AT24C02_Read(MINUTE_ADDR);
    second = AT24C02_Read(SECOND_ADDR);
    hourlyChime = AT24C02_Read(HOURLY_CHIME_ADDR) ? 1 : 0;
}

// �洢����
void SaveDate()
{
    AT24C02_Write(YEAR_ADDR_L, year % 256);
    AT24C02_Write(YEAR_ADDR_H, year / 256);
    AT24C02_Write(MONTH_ADDR, month);
    AT24C02_Write(DAY_ADDR, day);
}

// װ������
void LoadDate()
{
    year = 0;
    year += AT24C02_Read(YEAR_ADDR_L);
    year += AT24C02_Read(YEAR_ADDR_H) * 256;
    month = AT24C02_Read(MONTH_ADDR);
    day = AT24C02_Read(DAY_ADDR);
}

// �洢����
void SaveAlarm()
{
    AT24C02_Write(ALARM_CLOCK_HOUR_ADDR, alarmClockHour);
    AT24C02_Write(ALARM_CLOCK_MINUTE_ADDR, alarmClockMinute);
    AT24C02_Write(ALARM_CLOCK_WEEKDAY_ADDR, alarmClockWeekday);
    AT24C02_Write(ALARM_CLOCK_ADDR, alarmClock);
}

// װ������
void LoadAlarm()
{
    alarmClockHour = AT24C02_Read(ALARM_CLOCK_HOUR_ADDR);
    alarmClockMinute = AT24C02_Read(ALARM_CLOCK_MINUTE_ADDR);
    alarmClockWeekday = AT24C02_Read(ALARM_CLOCK_WEEKDAY_ADDR);
    alarmClock = AT24C02_Read(ALARM_CLOCK_ADDR) ? 1 : 0;
}

// ��ʾʱ��
void DisplayTime()
{
    line[0][4] = hour / 10 + '0'; // ʱʮλ
    line[0][5] = hour % 10 + '0'; // ʱ��λ
    line[0][6] = ':';
    line[0][7] = minute / 10 + '0'; // ��ʮλ
    line[0][8] = minute % 10 + '0'; // �ָ�λ
    line[0][9] = ':';
    line[0][10] = second / 10 + '0'; // ��ʮλ
    line[0][11] = second % 10 + '0'; // ���λ

    LCD12864_DisplayOneLine(0, line[0]);
}

// ��ʾ����
void DisplayDate()
{
    line[1][3] = year / 1000 + '0';     // ��ǧλ
    line[1][4] = year / 100 % 10 + '0'; // ���λ
    line[1][5] = year / 10 % 10 + '0';  // ��ʮλ
    line[1][6] = year % 10 + '0';       // ���λ
    line[1][7] = '-';
    line[1][8] = month / 10 + '0'; // ��ʮλ
    line[1][9] = month % 10 + '0'; // �¸�λ
    line[1][10] = '-';
    line[1][11] = day / 10 + '0'; // ��ʮλ
    line[1][12] = day % 10 + '0'; // �ո�λ

    LCD12864_DisplayOneLine(1, line[1]);
}

// ��ʾ����
void DisplayWeekday()
{
    switch (weekday)
    {
    case 0:
        strncpy(line[2] + 2, "����", 4);
        break;
    case 1:
        strncpy(line[2] + 2, "��һ", 4);
        break;
    case 2:
        strncpy(line[2] + 2, "�ܶ�", 4);
        break;
    case 3:
        strncpy(line[2] + 2, "��", 2);
        line[2][4] = 0xC8;
        line[2][5] = 0xFD;
        break;
    case 4:
        strncpy(line[2] + 2, "����", 4);
        break;
    case 5:
        strncpy(line[2] + 2, "����", 4);
        break;
    case 6:
        strncpy(line[2] + 2, "����", 4);
        break;
    default:
        break;
    }

    LCD12864_DisplayOneLine(2, line[2]);
}
// ��ʾ�¶�
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

// ��ʾ���㱨ʱ����
void DisplayHourlyChime()
{
    strncpy(line[3] + 2, "���㱨ʱ  ", 10);
    if (hourlyChime)
        strncpy(line[3] + 12, "��", 2);
    else
        strncpy(line[3] + 12, "��", 2);

    LCD12864_DisplayOneLine(3, line[3]);
}

// ������ڳ��򣬷���0��ʾ�������󣬷���1��ʾ��������
bit CheckDate(unsigned int year, unsigned char month, unsigned char day)
{
    if (month > 12 || month < 1)
        return 1; // �·�����

    if (day < 1)
        return 1; // ��������

    if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2)
    {
        if (day > 29)
            return 1; // ����2��29������
    }
    else
    {
        if (day > Seg_Date[month - 1])
            return 1;
    }

    return 0;
}

// ����ַ���
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

// ��ʱ��0�жϷ�����
void Timer0() interrupt 1
{
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1��
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
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "��", 2);
            else
                strncpy(line[2] + setAlarmClockWeekdayIndex * 2, "��", 2);
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
