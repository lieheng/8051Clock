#include <REG51.H>
#include "LCD12864.h"
#include "AT24C02.h"

#include <string.h>

#define INTERVAL 10 // ���10����

#define INIT_HOUR 0    // ��ʼ��Сʱ
#define INIT_MINUTE 0  // ��ʼ������
#define INIT_SECOND 58 // ��ʼ����

#define INIT_YEAR 2023 // ��ʼ����
#define INIT_MONTH 9   // ��ʼ����
#define INIT_DAY 19    // ��ʼ����

#define INIT_ALARM 1            // ��ʼ�����ӹ��ܣ�1��ʾ�죬0��ʾ����
#define INIT_ALARM_HOUR 0       // ��ʼ������Сʱ
#define INIT_ALARM_MINUTE 1     // ��ʼ�����ӷ���
#define INIT_ALARM_WEEKDAY 0x7B // ��ʼ����������
#define ALARMCLOCKTIMES 6       // �����������

#define INIT_HOURLY_CHIME 0 // ��ʼ�����㱨ʱ���ܣ�1��ʾ����0��ʾ��
#define HOURLYCHIMETIMES 3  // �����������

#define HOUR_ADDR 0x00   // Сʱ��AT24C02�еĴ洢��ַ
#define MINUTE_ADDR 0x01 // ������AT24C02�еĴ洢��ַ
#define SECOND_ADDR 0x02 // ����AT24C02�еĴ洢��ַ

#define YEAR_ADDR_H 0x03 // ��ĸ�λ��AT24C02�еĴ洢��ַ
#define YEAR_ADDR_L 0x04 // ��ĵ�λ��AT24C02�еĴ洢��ַ
#define MONTH_ADDR 0x05  // ����AT24C02�еĴ洢��ַ
#define DAY_ADDR 0x06    // ����AT24C02�еĴ洢��ַ

#define ALARM_HOUR_ADDR 0x07    // ����ʱ��AT24C02�еĴ洢��ַ
#define ALARM_MINUTE_ADDR 0x08  // ���ӷ���AT24C02�еĴ洢��ַ
#define ALARM_WEEKDAY_ADDR 0x09 // ����������AT24C02�еĴ洢��ַ

#define ALARM_ADDR 0x0A // ���ӹ�����AT24C02�еĴ洢��ַ

#define HOURLY_CHIME_ADDR 0x0B // ���㱨ʱ������AT24C02�еĴ洢��ַ

unsigned char xdata hour = INIT_HOUR;     // ��ʼ��Сʱ
unsigned char xdata minute = INIT_MINUTE; // ��ʼ������
unsigned char xdata second = INIT_SECOND; // ��ʼ����

unsigned int xdata year = INIT_YEAR;    // ��ʼ����
unsigned char xdata month = INIT_MONTH; // ��ʼ����
unsigned char xdata day = INIT_DAY;     // ��ʼ����

unsigned char xdata weekday = 0; // ���ڣ����ڳ�ʼ��������init()��������

unsigned char alarmHour = INIT_ALARM_HOUR;       // ����ʱ
unsigned char alarmMinute = INIT_ALARM_MINUTE;   // ���ӷ�
unsigned char alarmWeekday = INIT_ALARM_WEEKDAY; // �������ڹ��ܣ���0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

bit alarm = INIT_ALARM; // ���ӹ��ܣ�1��ʾ�죬0��ʾ����

bit hourlyChime = INIT_HOURLY_CHIME; // ���㱨ʱ���ܣ�1��ʾ����0��ʾ��

bit buttonDown = 0;            // �����ж��Ƿ��а������£�1Ϊ�У�0Ϊ��
bit button = 0;                // �����жϰ��µ����ĸ�����
unsigned int checkCount = 0;   // ���ڼ�鰴���ǳ������Ƕ̰�
unsigned char shortOrLang = 0; // ���ڱ�ʾ�����ǳ������Ƕ̰��ı�־��0��ʾ��Ч��1��ʾ�̰���2��ʾ����

unsigned char line1[17] = "                ";
unsigned char line2[17] = "                ";
unsigned char line3[17] = "                ";
unsigned char line4[17] = "                ";

unsigned char hourlyChimeTimes = 0; // ��¼���㱨ʱ���������
unsigned char alarmClockTimes = 0;  // ��¼���ӵ��������

unsigned int stopwatchMSecond = 0; // ���ĺ�����
unsigned char stopwatchSecond = 0; // ��������
unsigned char stopwatchMinute = 0; // ���ķ�����
unsigned int recordNum = 0;        // ��¼����

unsigned char xdata setAlarmHour = INIT_ALARM_HOUR;       // ��������ʱ�ı���
unsigned char xdata setAlarmMinute = INIT_ALARM_MINUTE;   // �������ӷֵı���
unsigned char xdata setAlarmWeekday = INIT_ALARM_WEEKDAY; // �����������ڵı�������0λΪ1��ʾ�����죬��1λΪ1��ʾ��һ�죬�Դ�����

unsigned char xdata setHour = INIT_HOUR;     // ����Сʱ�ı���
unsigned char xdata setMinute = INIT_MINUTE; // ���÷��ӵı���
unsigned char xdata setSecond = INIT_SECOND; // ������ı���

enum MODE
{
    SHOW,                     // ��ʾģʽ
    SET_YEAR,                 // ������ģʽ
    SET_MONTH,                // ������ģʽ
    SET_DAY,                  // ������ģʽ
    SET_HOUR,                 // ����ʱģʽ
    SET_MINUTE,               // ���÷�ģʽ
    SET_SECOND,               // ������ģʽ
    STOPWATCH,                // ���ģʽ
    STOPWATCH_START,          // ����ʱģʽ
    STOPWATCH_PAUSE,          // �����ͣģʽ
    ALARMCLOCK,               // ����ģʽ
    SET_ALARMCLOCK_HOUR,      // ��������ʱģʽ
    SET_ALARMCLOCK_MINUTE,    // �������ӷ�ģʽ
    SET_ALARMCLOCK_SUNDAY,    // ������������ģʽ
    SET_ALARMCLOCK_MONDAY,    // ����������һģʽ
    SET_ALARMCLOCK_TUESDAY,   // ���������ܶ�ģʽ
    SET_ALARMCLOCK_WEDNESDAY, // ������������ģʽ
    SET_ALARMCLOCK_THURSDAY,  // ������������ģʽ
    SET_ALARMCLOCK_FRIDAY,    // ������������ģʽ
    SET_ALARMCLOCK_SATURDAY   // ������������ģʽ
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

// ��ʼ������
void Init();
// �л�ģʽ
void ChangeMode(MODE);
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
// ����ַ���
void ClearChar(unsigned char *str);
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

    LCD12864_Init(); // ��ʼ��Һ����

    hour = AT24C02_Read(HOUR_ADDR);
    minute = AT24C02_Read(MINUTE_ADDR);
    second = AT24C02_Read(SECOND_ADDR);

    year = 0;
    year += AT24C02_Read(YEAR_ADDR_L);
    year += AT24C02_Read(YEAR_ADDR_H) * 256;
    month = AT24C02_Read(MONTH_ADDR);
    day = AT24C02_Read(DAY_ADDR);

    alarmHour = AT24C02_Read(ALARM_HOUR_ADDR);
    alarmMinute = AT24C02_Read(ALARM_MINUTE_ADDR);
    alarmWeekday = AT24C02_Read(ALARM_WEEKDAY_ADDR);

    alarm = AT24C02_Read(ALARM_ADDR) ? 1 : 0;
    hourlyChime = AT24C02_Read(HOURLY_CHIME_ADDR) ? 1 : 0;

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

void ChangeMode(MODE)
{
    unsigned char i = 0;
    mode = MODE;
    switch (mode)
    {
    case SHOW:
        ClearChar(line1);
        ClearChar(line2);
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

        strncpy(line4 + 2, "���㱨ʱ  ", 10);
        if (hourlyChime)
            strncpy(line4 + 12, "��", 2);
        else
            strncpy(line4 + 12, "��", 2);

        LCD12864_DisplayOneLine(LINE1, line1, 16);
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        LCD12864_DisplayOneLine(LINE4, line4, 16);

        UpdateWeekday();
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

        line1[4] = stopwatchMinute / 10 + '0'; // ����ʮλ
        line1[5] = stopwatchMinute % 10 + '0'; // ���Ӹ�λ
        line1[6] = ':';
        line1[7] = stopwatchSecond / 10 + '0'; // ��ʮλ
        line1[8] = stopwatchSecond % 10 + '0'; // ���λ
        line1[9] = ':';
        line1[10] = stopwatchMSecond / 100 + '0';     // �����λ
        line1[11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ

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
                strncpy(line3 + 2 * i, "��", 2);
            else
                strncpy(line3 + 2 * i, "��", 2);
        }
        if (alarm)
            strncpy(line3 + 14, "��", 2);
        else
            strncpy(line3 + 14, "��", 2);
        strcpy(line4, "��һ��");
        line4[6] = 0xC8;
        line4[7] = 0xFD;
        strcpy(line4 + 8, "��������");
        LCD12864_DisplayOneLine(LINE1, line1, 16);
        LCD12864_DisplayOneLine(LINE2, line2, 16);
        LCD12864_DisplayOneLine(LINE3, line3, 16);
        LCD12864_DisplayOneLine(LINE4, line4, 16);
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
                line2[0] = (recordNum % 1000) / 100 + '0';
                line2[1] = (recordNum % 100) / 10 + '0';
                line2[2] = (recordNum % 10) + '0';
                line2[4] = stopwatchMinute / 10 + '0'; // ����ʮλ
                line2[5] = stopwatchMinute % 10 + '0'; // ���Ӹ�λ
                line2[6] = ':';
                line2[7] = stopwatchSecond / 10 + '0'; // ��ʮλ
                line2[8] = stopwatchSecond % 10 + '0'; // ���λ
                line2[9] = ':';
                line2[10] = stopwatchMSecond / 100 + '0';     // �����λ
                line2[11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ

                LCD12864_DisplayOneLine(LINE2, line2, 16);
            }
            else if (recordNum % 3 == 2)
            {
                ClearChar(line3);
                line3[0] = (recordNum % 1000) / 100 + '0';
                line3[1] = (recordNum % 100) / 10 + '0';
                line3[2] = (recordNum % 10) + '0';
                line3[4] = stopwatchMinute / 10 + '0'; // ����ʮλ
                line3[5] = stopwatchMinute % 10 + '0'; // ���Ӹ�λ
                line3[6] = ':';
                line3[7] = stopwatchSecond / 10 + '0'; // ��ʮλ
                line3[8] = stopwatchSecond % 10 + '0'; // ���λ
                line3[9] = ':';
                line3[10] = stopwatchMSecond / 100 + '0';     // �����λ
                line3[11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ
                LCD12864_DisplayOneLine(LINE3, line3, 16);
            }
            else
            {
                ClearChar(line4);
                line4[0] = (recordNum % 1000) / 100 + '0';
                line4[1] = (recordNum % 100) / 10 + '0';
                line4[2] = (recordNum % 10) + '0';
                line4[4] = stopwatchMinute / 10 + '0'; // ����ʮλ
                line4[5] = stopwatchMinute % 10 + '0'; // ���Ӹ�λ
                line4[6] = ':';
                line4[7] = stopwatchSecond / 10 + '0'; // ��ʮλ
                line4[8] = stopwatchSecond % 10 + '0'; // ���λ
                line4[9] = ':';
                line4[10] = stopwatchMSecond / 100 + '0';     // �����λ
                line4[11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ
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
            line2[3] = year / 1000 + '0';     // ���ǧλ
            line2[4] = year / 100 % 10 + '0'; // ��ݰ�λ
            line2[5] = year / 10 % 10 + '0';  // ���ʮλ
            line2[6] = year % 10 + '0';       // ��ݸ�λ
            ChangeMode(SET_MONTH);
            break;
        case SET_MONTH:
            line2[8] = month / 10 + '0'; // �·�ʮλ
            line2[9] = month % 10 + '0'; // �·ݸ�λ
            ChangeMode(SET_DAY);
            break;
        case SET_DAY:
            line2[11] = day / 10 + '0'; // ����ʮλ
            line2[12] = day % 10 + '0'; // ���ڸ�λ
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
                strncpy(line3 + 0, "��", 2);
            else
                strncpy(line3 + 0, "��", 2);
            ChangeMode(SET_ALARMCLOCK_MONDAY);
            break;
        case SET_ALARMCLOCK_MONDAY:
            if (setAlarmWeekday & (1 << 1))
                strncpy(line3 + 2, "��", 2);
            else
                strncpy(line3 + 2, "��", 2);
            ChangeMode(SET_ALARMCLOCK_TUESDAY);
            break;
        case SET_ALARMCLOCK_TUESDAY:
            if (setAlarmWeekday & (1 << 2))
                strncpy(line3 + 4, "��", 2);
            else
                strncpy(line3 + 4, "��", 2);
            ChangeMode(SET_ALARMCLOCK_WEDNESDAY);
            break;
        case SET_ALARMCLOCK_WEDNESDAY:
            if (setAlarmWeekday & (1 << 3))
                strncpy(line3 + 6, "��", 2);
            else
                strncpy(line3 + 6, "��", 2);
            ChangeMode(SET_ALARMCLOCK_THURSDAY);
            break;
        case SET_ALARMCLOCK_THURSDAY:
            if (setAlarmWeekday & (1 << 4))
                strncpy(line3 + 8, "��", 2);
            else
                strncpy(line3 + 8, "��", 2);
            ChangeMode(SET_ALARMCLOCK_FRIDAY);
            break;
        case SET_ALARMCLOCK_FRIDAY:
            if (setAlarmWeekday & (1 << 5))
                strncpy(line3 + 10, "��", 2);
            else
                strncpy(line3 + 10, "��", 2);
            ChangeMode(SET_ALARMCLOCK_SATURDAY);
            break;
        case SET_ALARMCLOCK_SATURDAY:
            if (setAlarmWeekday & (1 << 6))
                strncpy(line3 + 12, "��", 2);
            else
                strncpy(line3 + 12, "��", 2);
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
            strncpy(line4 + 2, "���㱨ʱ  ", 10);
            if (hourlyChime)
            {
                AT24C02_Write(HOURLY_CHIME_ADDR, 0x01);
                strncpy(line4 + 12, "��", 2);
            }
            else
            {
                AT24C02_Write(HOURLY_CHIME_ADDR, 0x00);
                strncpy(line4 + 12, "��", 2);
            }

            LCD12864_DisplayOneLine(LINE4, line4, 16);
            break;
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
            if (alarm)
            {
                AT24C02_Write(ALARM_ADDR, 0x01);
                strncpy(line3 + 14, "��", 2);
            }

            else
            {
                AT24C02_Write(ALARM_ADDR, 0x00);
                strncpy(line3 + 14, "��", 2);
            }

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

            setAlarmWeekday = setAlarmWeekday ^ (1 << 0); // ��0λȡ��

            break;
        case SET_ALARMCLOCK_MONDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 1); // ��1λȡ��

            break;
        case SET_ALARMCLOCK_TUESDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 2); // ��2λȡ��

            break;
        case SET_ALARMCLOCK_WEDNESDAY:;
            setAlarmWeekday = setAlarmWeekday ^ (1 << 3); // ��3λȡ��

            break;
        case SET_ALARMCLOCK_THURSDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 4); // ��4λȡ��

            break;
        case SET_ALARMCLOCK_FRIDAY:

            setAlarmWeekday = setAlarmWeekday ^ (1 << 5); // ��5λȡ��

            break;
        case SET_ALARMCLOCK_SATURDAY:

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
            setHour = hour;
            setMinute = minute;
            setSecond = second;
            ChangeMode(SET_HOUR);
            break;
        case SET_YEAR:
        case SET_MONTH:
        case SET_DAY:
            if (!CheckDate(year, month, day)) // ��������������ȷ��
            {
                AT24C02_Write(YEAR_ADDR_L, year % 256);
                AT24C02_Write(YEAR_ADDR_H, year / 256);
                AT24C02_Write(MONTH_ADDR, month);
                AT24C02_Write(DAY_ADDR, day);
                ChangeMode(SHOW);
            }
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            hour = setHour;
            minute = setMinute;
            second = setSecond;
            AT24C02_Write(HOUR_ADDR, hour);
            AT24C02_Write(MINUTE_ADDR, minute);
            AT24C02_Write(SECOND_ADDR, second);
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
            AT24C02_Write(ALARM_HOUR_ADDR, alarmHour);
            AT24C02_Write(ALARM_MINUTE_ADDR, alarmMinute);
            AT24C02_Write(ALARM_WEEKDAY_ADDR, alarmWeekday);
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
    weekday = (month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1;
    while (weekday < 0)
    {
        weekday += 7;
    }
    weekday %= 7;
    ClearChar(line3);

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
        strncpy(line3 + 6, "��", 2);
        line3[8] = 0xC8;
        line3[9] = 0xFD;
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

    LCD12864_DisplayOneLine(LINE3, line3, 16);
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
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == (1000 / INTERVAL))
    { // 1��
        interruptCount = 0;
        SecondIncrease();
        AT24C02_Write(HOUR_ADDR, hour);
        AT24C02_Write(MINUTE_ADDR, minute);
        AT24C02_Write(SECOND_ADDR, second);
        if (mode == SHOW)
        {
            ClearChar(line1);

            line1[4] = hour / 10 + '0'; // Сʱʮλ
            line1[5] = hour % 10 + '0'; // Сʱ��λ
            line1[6] = ':';
            line1[7] = minute / 10 + '0'; // ����ʮλ
            line1[8] = minute % 10 + '0'; // ���Ӹ�λ
            line1[9] = ':';
            line1[10] = second / 10 + '0'; // ����ʮλ
            line1[11] = second % 10 + '0'; // ���Ӹ�λ

            LCD12864_DisplayOneLine(LINE1, line1, 16);
        }
        if (hour == 0 && minute == 0 && second == 0)
        {
            DateIncrease();
            UpdateWeekday();
            AT24C02_Write(YEAR_ADDR_L, year % 256);
            AT24C02_Write(YEAR_ADDR_H, year / 256);
            AT24C02_Write(MONTH_ADDR, month);
            AT24C02_Write(DAY_ADDR, day);
            if (mode == SHOW)
            {
                ClearChar(line2);

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

                LCD12864_DisplayOneLine(LINE2, line2, 16);
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
            line2[3] = year / 1000 + '0';     // ���ǧλ
            line2[4] = year / 100 % 10 + '0'; // ��ݰ�λ
            line2[5] = year / 10 % 10 + '0';  // ���ʮλ
            line2[6] = year % 10 + '0';       // ��ݸ�λ
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
            line2[8] = month / 10 + '0'; // �·�ʮλ
            line2[9] = month % 10 + '0'; // �·ݸ�λ
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
            line2[11] = day / 10 + '0'; // ����ʮλ
            line2[12] = day % 10 + '0'; // ���ڸ�λ
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
        line1[4] = stopwatchMinute / 10 + '0'; // ����ʮλ
        line1[5] = stopwatchMinute % 10 + '0'; // ���Ӹ�λ
        line1[6] = ':';
        line1[7] = stopwatchSecond / 10 + '0'; // ��ʮλ
        line1[8] = stopwatchSecond % 10 + '0'; // ���λ
        line1[9] = ':';
        line1[10] = stopwatchMSecond / 100 + '0';     // �����λ
        line1[11] = stopwatchMSecond / 10 % 10 + '0'; // ����ʮλ

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
                strncpy(line3 + 0, "��", 2);
            else
                strncpy(line3 + 0, "��", 2);
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
                strncpy(line3 + 2, "��", 2);
            else
                strncpy(line3 + 2, "��", 2);
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
                strncpy(line3 + 4, "��", 2);
            else
                strncpy(line3 + 4, "��", 2);
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
                strncpy(line3 + 6, "��", 2);
            else
                strncpy(line3 + 6, "��", 2);
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
                strncpy(line3 + 8, "��", 2);
            else
                strncpy(line3 + 8, "��", 2);
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
                strncpy(line3 + 10, "��", 2);
            else
                strncpy(line3 + 10, "��", 2);
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
                strncpy(line3 + 12, "��", 2);
            else
                strncpy(line3 + 12, "��", 2);
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
