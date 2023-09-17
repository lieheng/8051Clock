#include <REG51.H>

#define INTERVAL 1 // ���1����

#define INIT_HOUR 23   // ��ʼ��Сʱ
#define INIT_MINUTE 59 // ��ʼ������
#define INIT_SECOND 58 // ��ʼ����

#define HOURLYCHIMETIMES 3 // �����������
#define ALARMCLOCKTIMES 6  // �����������

unsigned char hour = INIT_HOUR;     // ��ʼ��Сʱ
unsigned char minute = INIT_MINUTE; // ��ʼ������
unsigned char second = INIT_SECOND; // ��ʼ����

unsigned int year = 2020; // ��ʼ����
unsigned char month = 2;  // ��ʼ����
unsigned char day = 28;   // ��ʼ����

unsigned char weekDay = 5; // ��ʼ������

unsigned char alarmHour = 23;   // ����ʱ
unsigned char alarmMinute = 59; // ���ӷ�

enum MODE
{
    SHOW_TIME,        // ��ʾʱ��ģʽ
    SHOW_DATE,        // ��ʾ����ģʽ
    SET_YEAR,         // ������ģʽ
    SET_MONTH,        // ������ģʽ
    SET_DAY,          // ������ģʽ
    SET_HOUR,         // ����ʱģʽ
    SET_MINUTE,       // ���÷�ģʽ
    SET_SECOND,       // ������ģʽ
    STOPWATCH,        // ���ģʽ
    STOPWATCH_START,  // ����ʱģʽ
    STOPWATCH_PAUSE,  // �����ͣģʽ
    ALARMCLOCK,       // ����ģʽ
    ALARMCLOCK_HOUR,  // ��������ʱģʽ
    ALARMCLOCK_MINUTE // �������ӷ�ģʽ
};

unsigned char mode = SHOW_TIME; // ģʽ

unsigned char displayIndex = 0;
unsigned char LED8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char LED8Point = 0;

unsigned int interruptCount = 0; // �жϴ���

bit hourlyChime = 0; // ���㱨ʱ���ܣ�1��ʾ����0��ʾ��
bit alarm = 0;       // ���ӹ��ܣ�1��ʾ�죬0��ʾ����

sbit SEG_DS = P2 ^ 0;   // 74HC595оƬ����������
sbit SEG_SHCP = P2 ^ 1; // 74HC595оƬ�Ŀ������ţ���������������
sbit SEG_STCP = P2 ^ 2; // 74HC595оƬ�Ŀ������ţ������ظ�������

sbit Chime = P2 ^ 3; // ������

unsigned char code Seg_Data[] = {
    // ��������ܵı��룬�������ݶ�����CODE��
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
    0xFF, /*OFF*/
};

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

unsigned char code Seg_Addr[] = {
    // �����λѡ���룬������ʾ8λ�еĵڼ�λ
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

void Display(unsigned char numEnable, unsigned char pointEnable);
void SEG_Send595OneByte(unsigned char ucData); // ��74HC595д��һ��8λ������
void SecondIncrease();

unsigned int checkCount = 0;
unsigned char shortOrLang = 0; // 0��ʾ��Ч��1��ʾ�̰���2��ʾ����

bit buttonDown = 0;
bit button = 0;

sbit P3_2 = P3 ^ 2;
sbit P3_3 = P3 ^ 3;

// �жϰ����ǳ������Ƕ̰�
void Check()
{
    checkCount++;
    if (checkCount < 10 / INTERVAL) // 10ms�ӳ�ȥ����
        return;

    if (checkCount > 1010 / INTERVAL) // �������һ�룬Ϊ����
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

// ������ڳ��򣬷���0��ʾ�������󣬷���1��ʾ��������
int CheckDate(unsigned int year, unsigned char month, unsigned char day)
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

// �̰��������
void ShortPress()
{
    if (button == 0) // ��������Ǽ�A
    {
        switch (mode)
        {
        case SHOW_TIME:
            mode = STOPWATCH;
            break;
        case STOPWATCH:
            mode = ALARMCLOCK;
            break;
        case ALARMCLOCK:
            mode = SHOW_TIME;
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
            mode = ALARMCLOCK_HOUR;
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
        unsigned int setYear = LED8[0] * 1000 + LED8[1] * 100 + LED8[2] * 10 + LED8[3];
        unsigned char setMonth = LED8[4] * 10 + LED8[5];
        unsigned char setDay = LED8[6] * 10 + LED8[7];
        unsigned char setAlarmHour = LED8[3] * 10 + LED8[4];
        unsigned char setAlarmMinute = LED8[6] * 10 + LED8[7];
        switch (mode)
        {
        case SHOW_TIME:
            mode = SHOW_DATE;
            break;
        case SHOW_DATE:
            mode = SHOW_TIME;
            break;
        case SET_YEAR:
            setYear++;
            setYear %= 10000;
            LED8[0] = setYear / 1000;
            LED8[1] = setYear / 100 % 10;
            LED8[2] = setYear / 10 % 10;
            LED8[3] = setYear % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
        case SET_MONTH:
            setMonth = (setMonth % 12) + 1;
            LED8[4] = setMonth / 10;
            LED8[5] = setMonth % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
        case SET_DAY:
            if (((setYear % 4 == 0 && setYear % 100 != 0) || (setYear % 400 == 0)) && setMonth == 2) // ����2��
                setDay = (setDay % 29) + 1;
            else
                setDay = (setDay % Seg_Date[setMonth - 1]) + 1;
            LED8[6] = setDay / 10;
            LED8[7] = setDay % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
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
        case ALARMCLOCK:
            alarm = !alarm;
            break;
        case ALARMCLOCK_HOUR:
            setAlarmHour++;
            setAlarmHour %= 24;
            LED8[3] = setAlarmHour / 10;
            LED8[4] = setAlarmHour % 10;
            break;
        case ALARMCLOCK_MINUTE:
            setAlarmMinute++;
            setAlarmMinute %= 60;
            LED8[6] = setAlarmMinute / 10;
            LED8[7] = setAlarmMinute % 10;
            break;
        default:
            break;
        }
    }

    shortOrLang = 0;
}

// �����������
void LongPress()
{
    if (button == 0) // ��������Ǽ�A
    {
        switch (mode)
        {
        case SHOW_TIME:
            mode = SET_HOUR;
            break;
        case SHOW_DATE:
            mode = SET_YEAR;
            break;
        case SET_YEAR:
        case SET_MONTH:
        case SET_DAY:
            if (!CheckDate(LED8[0] * 1000 + LED8[1] * 100 + LED8[2] * 10 + LED8[3], LED8[4] * 10 + LED8[5], LED8[6] * 10 + LED8[7])) // ��������������ȷ��
            {
                year = LED8[0] * 1000 + LED8[1] * 100 + LED8[2] * 10 + LED8[3];
                month = LED8[4] * 10 + LED8[5];
                day = LED8[6] * 10 + LED8[7];

                weekDay = ((month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1) % 7;

                mode = SHOW_DATE;
            }
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            hour = LED8[0] * 10 + LED8[1];
            minute = LED8[3] * 10 + LED8[4];
            second = LED8[6] * 10 + LED8[7];

            mode = SHOW_TIME;
            break;
        case ALARMCLOCK:
            mode = ALARMCLOCK_HOUR;
            break;
        case ALARMCLOCK_HOUR:
        case ALARMCLOCK_MINUTE:
            alarmHour = LED8[3] * 10 + LED8[4];
            alarmMinute = LED8[6] * 10 + LED8[7];

            mode = ALARMCLOCK;
            break;
        default:
            break;
        }
    }
    else
    {
        unsigned int setYear = LED8[0] * 1000 + LED8[1] * 100 + LED8[2] * 10 + LED8[3];
        unsigned char setMonth = LED8[4] * 10 + LED8[5];
        unsigned char setDay = LED8[6] * 10 + LED8[7];
        switch (mode)
        {
        case SHOW_TIME:
            hourlyChime = !hourlyChime;
            break;
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            mode = SHOW_TIME;
            break;
        case SET_YEAR:
            setYear--;
            setYear %= 10000;
            LED8[0] = setYear / 1000;
            LED8[1] = setYear / 100 % 10;
            LED8[2] = setYear / 10 % 10;
            LED8[3] = setYear % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
        case SET_MONTH:
            setMonth += 10;
            setMonth = (setMonth % 12) + 1;
            LED8[4] = setMonth / 10;
            LED8[5] = setMonth % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
        case SET_DAY:
            if (((setYear % 4 == 0 && setYear % 100 != 0) || (setYear % 400 == 0)) && setMonth == 2)
            {
                setDay += 29 - 2;
                setDay = (setDay % 29) + 1;
            }
            else
            {
                setDay += Seg_Date[setMonth - 1] - 2;
                setDay = (setDay % Seg_Date[setMonth - 1]) + 1;
            }
            LED8[6] = setDay / 10;
            LED8[7] = setDay % 10;
            weekDay = ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) + ((setMonth > 2 ? (setYear % 100) : (setYear % 100) - 1) / 4) + (setYear / 100) / 4 - 2 * (setYear / 100) + (26 * ((setMonth > 2 ? setMonth : setMonth + 12) + 1) / 10) + setDay - 1) % 7;
            break;
        case STOPWATCH_PAUSE:
            mode = STOPWATCH;
            break;
        case ALARMCLOCK_HOUR:
        case ALARMCLOCK_MINUTE:
            mode = ALARMCLOCK;
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

unsigned char hourlyChimeTimes = 0;
unsigned char alarmClockTimes = 0;

unsigned int stopwatchMSecond = 0;
unsigned char stopwatchSecond = 0;
unsigned char stopwatchMinute = 0;

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
        if (hour == 0 && minute == 0 && second == 0)
        {
            DateIncrease();
            weekDay = ((month > 2 ? (year % 100) : (year % 100) - 1) + ((month > 2 ? (year % 100) : (year % 100) - 1) / 4) + (year / 100) / 4 - 2 * (year / 100) + (26 * ((month > 2 ? month : month + 12) + 1) / 10) + day - 1) % 7;
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

    if (mode == SHOW_TIME && alarm == 1)
    {
        if (hour == alarmHour && minute == alarmMinute && second == 00 && interruptCount == 0)
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

    if (mode == SHOW_TIME && hourlyChime == 1)
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
    case SHOW_TIME:
        LED8[0] = hour / 10; // ��ʾСʱʮλ
        LED8[1] = hour % 10; // ��ʾСʱ��λ
        LED8[2] = 16;
        LED8[3] = minute / 10; // ��ʾ����ʮλ
        LED8[4] = minute % 10; // ��ʾ���Ӹ�λ
        LED8[5] = 16;
        LED8[6] = second / 10; // ��ʾ��ʮλ
        LED8[7] = second % 10; // ��ʾ���λ
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        Display(0xFF, 0xFF);
        break;
    case SHOW_DATE:
        LED8[0] = year / 1000;
        LED8[1] = year / 100 % 10;
        LED8[2] = year / 10 % 10;
        LED8[3] = year % 10;
        LED8[4] = month / 10;
        LED8[5] = month % 10;
        LED8[6] = day / 10;
        LED8[7] = day % 10;
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        Display(0xFF, 0xFF);
        break;
    case SET_HOUR:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0xFC, 0xFF);
        break;
    case SET_MINUTE:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0xE7, 0xFF);
        break;
    case SET_SECOND:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0x3F, 0xFF);
        break;
    case SET_YEAR:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0xF0, 0xFF);
        break;
    case SET_MONTH:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0xCF, 0xFF);
        break;
    case SET_DAY:
        LED8Point = (1 << weekDay) | (hourlyChime ? 0x80 : 0);
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0x3F, 0xFF);
        break;
    case STOPWATCH:
        stopwatchMSecond = 0;
        stopwatchSecond = 0;
        stopwatchMinute = 0;
        LED8[0] = stopwatchMinute / 10; // ��ʾ����ʮλ
        LED8[1] = stopwatchMinute % 10; // ��ʾ���Ӹ�λ
        LED8[2] = 16;
        LED8[3] = stopwatchSecond / 10; // ��ʾ��ʮλ
        LED8[4] = stopwatchSecond % 10; // ��ʾ���λ
        LED8[5] = 16;
        LED8[6] = stopwatchMSecond / 100;
        LED8[7] = (stopwatchMSecond % 100) / 10;

        LED8Point = 0;
        Display(0xFF, 0xFF);
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

        LED8[0] = stopwatchMinute / 10; // ��ʾ����ʮλ
        LED8[1] = stopwatchMinute % 10; // ��ʾ���Ӹ�λ
        LED8[2] = 16;
        LED8[3] = stopwatchSecond / 10; // ��ʾ��ʮλ
        LED8[4] = stopwatchSecond % 10; // ��ʾ���λ
        LED8[5] = 16;
        LED8[6] = stopwatchMSecond / 100;
        LED8[7] = (stopwatchMSecond % 100) / 10;

        LED8Point = 0;
        Display(0xFF, 0xFF);
        break;
    case STOPWATCH_PAUSE:
        LED8Point = 0;
        Display(0xFF, 0xFF);
        break;
    case ALARMCLOCK:
        LED8[3] = alarmHour / 10;
        LED8[4] = alarmHour % 10;
        LED8[5] = 16;
        LED8[6] = alarmMinute / 10;
        LED8[7] = alarmMinute % 10;
        if (alarm)
        {
            LED8[0] = 17;
            LED8[1] = 17;
            LED8[2] = 17;
        }
        else
        {
            LED8[0] = 0;
            LED8[1] = 15;
            LED8[2] = 15;
        }
        LED8Point = 0;
        Display(0xFF, 0xFF);
        break;
    case ALARMCLOCK_HOUR:
        LED8Point = 0;
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0xE7, 0xFF);
        break;
    case ALARMCLOCK_MINUTE:
        LED8Point = 0;
        if (interruptCount < (500 / INTERVAL))
            Display(0xFF, 0xFF);
        else
            Display(0x3F, 0xFF);
        break;
    default:
        break;
    }
}

void Init()
{
    Chime = 0;

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

void main()
{
    Init();
    while (1)
    {
        ;
    }
}

void Display(unsigned char numEnable, unsigned char pointEnable)
{
    unsigned char LED = Seg_Data[17];

    if (((numEnable >> displayIndex) & 1) == 1)
    {
        LED = Seg_Data[LED8[displayIndex]];
    }

    if (((pointEnable >> displayIndex) & 1) == 1 && ((LED8Point >> displayIndex) & 1) == 1)
    {
        LED = LED & 0x7F; // ��ʾС����
    }

    SEG_Send595OneByte(Seg_Addr[displayIndex]); // ��ʾ����һ���������
    SEG_Send595OneByte(LED);

    SEG_STCP = 0;
    SEG_STCP = 1; // STCP���ŵ������ظ�������
    SEG_STCP = 0;

    displayIndex++;
    displayIndex %= 8;
}

// ��HC595����һ���ֽ�
void SEG_Send595OneByte(unsigned char ucData)
{
    unsigned char i;

    for (i = 0; i < 8; i++) // 8λ��������д�룬��д���λ
    {
        SEG_DS = (ucData & 0x80); // �ȶ����λ   x&0x80;
        SEG_SHCP = 0;
        SEG_SHCP = 1;
        SEG_SHCP = 0; // SHCP���ŵ���������������
        ucData <<= 1; // ��������
    }
}