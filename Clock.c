#include <REG51.H>
#include "LCD12864.h"

#include <string.h>

#define INTERVAL 10 // ���10����

#define INIT_HOUR 23   // ��ʼ��Сʱ
#define INIT_MINUTE 59 // ��ʼ������
#define INIT_SECOND 58 // ��ʼ����

#define INIT_YEAR 2020 // ��ʼ����
#define INIT_MONTH 2   // ��ʼ����
#define INIT_DAY 28    // ��ʼ����

#define INIT_ALARM 0         // ��ʼ�����ӹ��ܣ�1��ʾ�죬0��ʾ����
#define INIT_ALARM_HOUR 23   // ��ʼ������Сʱ
#define INIT_ALARM_MINUTE 59 // ��ʼ�����ӷ���
#define INIT_ALARM_WEEKDAY 0 // ��ʼ����������
#define ALARMCLOCKTIMES 6    // �����������

#define INIT_HOURLY_CHIME 0 // ��ʼ�����㱨ʱ���ܣ�1��ʾ����0��ʾ��
#define HOURLYCHIMETIMES 3  // �����������

unsigned char hour = INIT_HOUR;     // ��ʼ��Сʱ
unsigned char minute = INIT_MINUTE; // ��ʼ������
unsigned char second = INIT_SECOND; // ��ʼ����

unsigned int year = INIT_YEAR;    // ��ʼ����
unsigned char month = INIT_MONTH; // ��ʼ����
unsigned char day = INIT_DAY;     // ��ʼ����

unsigned char weekday = 0; // ���ڣ����ڳ�ʼ��������init()��������

unsigned char alarmHour = INIT_ALARM_HOUR;       // ����ʱ
unsigned char alarmMinute = INIT_ALARM_MINUTE;   // ���ӷ�
unsigned char alarmWeekday = INIT_ALARM_WEEKDAY; // �������ڹ��ܣ���0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

bit alarm = INIT_ALARM; // ���ӹ��ܣ�1��ʾ�죬0��ʾ����

bit hourlyChime = INIT_HOURLY_CHIME; // ���㱨ʱ���ܣ�1��ʾ����0��ʾ��

bit buttonDown = 0;            // �����ж��Ƿ��а������£�1Ϊ�У�0Ϊ��
bit button = 0;                // �����жϰ��µ����ĸ�����
unsigned int checkCount = 0;   // ���ڼ�鰴���ǳ������Ƕ̰���ÿ��һ����ñ�����һ
unsigned char shortOrLang = 0; // ���ڱ�ʾ�����ǳ������Ƕ̰��ı�־��0��ʾ��Ч��1��ʾ�̰���2��ʾ����

unsigned char line1[16] = "                ";
unsigned char line2[16] = "                ";
unsigned char line3[16] = "                ";
unsigned char line4[16] = "                ";

unsigned char hourlyChimeTimes = 0; // ��¼���㱨ʱ���������
unsigned char alarmClockTimes = 0;  // ��¼���ӵ��������

unsigned int stopwatchMSecond = 0; // ���ĺ�����
unsigned char stopwatchSecond = 0; // ��������
unsigned char stopwatchMinute = 0; // ���ķ�����

unsigned char setAlarmHour = INIT_ALARM_HOUR;       // ��������ʱ�ı���
unsigned char setAlarmMinute = INIT_ALARM_MINUTE;   // �������ӷֵı���
unsigned char setAlarmWeekday = INIT_ALARM_WEEKDAY; // �����������ڵı�������0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

enum MODE
{
    SHOW,                 // ��ʾģʽ
    SET_YEAR,             // ������ģʽ
    SET_MONTH,            // ������ģʽ
    SET_DAY,              // ������ģʽ
    SET_HOUR,             // ����ʱģʽ
    SET_MINUTE,           // ���÷�ģʽ
    SET_SECOND,           // ������ģʽ
    STOPWATCH,            // ���ģʽ
    STOPWATCH_START,      // ����ʱģʽ
    STOPWATCH_PAUSE,      // �����ͣģʽ
    ALARMCLOCK,           // ����ģʽ
    ALARMCLOCK_HOUR,      // ��������ʱģʽ
    ALARMCLOCK_MINUTE,    // �������ӷ�ģʽ
    ALARMCLOCK_SUNDAY,    // ������������ģʽ
    ALARMCLOCK_MONDAY,    // ����������һģʽ
    ALARMCLOCK_TUESDAY,   // ���������ܶ�ģʽ
    ALARMCLOCK_WEDNESDAY, // ������������ģʽ
    ALARMCLOCK_THURSDAY,  // ������������ģʽ
    ALARMCLOCK_FRIDAY,    // ������������ģʽ
    ALARMCLOCK_SATURDAY   // ������������ģʽ
};

unsigned char mode = SHOW; // ģʽ

unsigned int interruptCount = 0; // �жϴ���

sbit Chime = P2 ^ 3; // ������

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

// ��ʼ������
void Init();
// ��鵱ǰ�����ǳ������Ƕ̰�
void Check();
// �����̰��������
void ShortPress();
// ���������������
void LongPress();
// �������Ӵ������
void SecondIncrease();
// �������Ӵ������
void DateIncrease();
// ��������
void UpdateWeekday();
// ������ڳ��򣬷���0��ʾ�������󣬷���1��ʾ��������
bit CheckDate(unsigned int year, unsigned char month, unsigned char day);

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

// ��ʼ������
void Init()
{

    UpdateWeekday(); // ��ʼ������

    LCD12864_Init(); // ��ʼ��Һ����

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

// ��鵱ǰ�����ǳ������Ƕ̰�
void Check()
{
    checkCount++; // ����ʱ�������ÿ��һ�����һ��

    if (checkCount < 10 / INTERVAL) // 10ms�ӳ�ȥ����
        return;

    if (checkCount > 1010 / INTERVAL) // �������ʱ�����һ�룬Ϊ������
    {
        shortOrLang = 2;
        checkCount = 0;
        buttonDown = 0;
    }
    else // �������ʱ��С��һ�룬���ж��Ƿ��ɿ������ɿ�����Ϊ�̰����������������
    {
        if ((button == 0 && P3_2 == 1) || (button == 1 && P3_3 == 1))
        {
            shortOrLang = 1;
            checkCount = 0;
            buttonDown = 0;
        }
    }
}

// �����̰��������
void ShortPress()
{
    if (button == 0) // ��������Ǽ�A
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
            if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2) // ����2��
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

            setAlarmWeekday = setAlarmWeekday ^ (1 << 0); // ��0λȡ��

            break;
        case ALARMCLOCK_MONDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 1); // ��1λȡ��

            break;
        case ALARMCLOCK_TUESDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 2); // ��2λȡ��

            break;
        case ALARMCLOCK_WEDNESDAY:;
            setAlarmWeekday = setAlarmWeekday ^ (1 << 3); // ��3λȡ��

            break;
        case ALARMCLOCK_THURSDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 4); // ��4λȡ��

            break;
        case ALARMCLOCK_FRIDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 5); // ��5λȡ��

            break;
        case ALARMCLOCK_SATURDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 6); // ��6λȡ��

            break;
        default:
            break;
        }
    }

    shortOrLang = 0; // ���ð�����־
}

// ���������������
void LongPress()
{
    if (button == 0) // ��������Ǽ�A
    {
        switch (mode)
        {
        case SHOW:
            mode = SET_HOUR;
            break;
        case SET_YEAR:
        case SET_MONTH:
        case SET_DAY:
            if (!CheckDate(year, month, day)) // ��������������ȷ��
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

    shortOrLang = 0; // ���ð�����־
}

// �������Ӵ������
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

// �������Ӵ������
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
    weekday = ((month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1) % 7;
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
    unsigned char *p;
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1��
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
            hourlyChimeTimes = 2 * (HOURLYCHIMETIMES + 1); // ��Ҳ��֪������ΪʲôҪ��һ������ʵ����������HOURLYCHIMETIMES-1�Σ�����Ҫ��һ���ϡ�
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
        line1[4] = hour / 10 + '0'; // Сʱʮλ
        line1[5] = hour % 10 + '0'; // Сʱ��λ
        line1[6] = ':';
        line1[7] = minute / 10 + '0'; // ����ʮλ
        line1[8] = minute % 10 + '0'; // ���Ӹ�λ
        line1[9] = ':';
        line1[10] = second / 10 + '0'; // ����ʮλ
        line1[11] = second % 10 + '0'; // ���Ӹ�λ

        line2[3] = year / 1000 + '0';     // ���ǧλ
        line2[4] = year / 100 % 10 + '0'; // ��ݰ�λ
        line2[5] = year / 10 % 10 + '0';  // ���ʮλ
        line2[6] = year % 10 + '0';       // ��ݸ�λ
        line2[7] = '-';
        line2[8] = month / 10 + '0'; // �·�ʮλ
        line2[9] = month % 10 + '0'; // �·ݸ�λ
        line2[10] = '-';
        line2[11] = day / 10 + '0'; // ����ʮλ
        line2[12] = day % 10 + '0'; // ���ڸ�λ

        switch (weekday)
        {
        case 0:
            strncpy(line3 + 6, "����", 4);
            break;
        case 1:
            strncpy(line3 + 6, "��һ", 4);
            break;
        case 2:
            strncpy(line3 + 6, "�ܶ�", 4);
            break;
        case 3:
            strncpy(line3 + 6, "����", 4);
            break;
        case 4:
            strncpy(line3 + 6, "����", 4);
            break;
        case 5:
            strncpy(line3 + 6, "����", 4);
            break;
        case 6:
            strncpy(line3 + 6, "����", 4);
            break;
        default:
            break;
        }
        strncpy(line4 + 2, "���㱨ʱ  ", 10);
        if (hourlyChime)
            strncpy(line4 + 12, "��", 2);
        else
            strncpy(line4 + 12, "��", 2);

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
