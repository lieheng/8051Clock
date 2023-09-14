#include <REG51.H>

#define INTERVAL 1 // ���1����

#define INIT_HOUR 23   // ��ʼ��Сʱ
#define INIT_MINUTE 59 // ��ʼ������
#define INIT_SECOND 20 // ��ʼ����

unsigned char hour = INIT_HOUR;     // ��ʼ��Сʱ
unsigned char minute = INIT_MINUTE; // ��ʼ������
unsigned char second = INIT_SECOND; // ��ʼ����

enum MODE
{
    SHOW,
    SET,
    SET_HOUR,
    SET_MINUTE,
    SET_SECOND,
    STOPWATCH,
    ALARMCLOCK
};

unsigned char mode = SHOW; // ģʽ��0Ϊ��ʾģʽ��1Ϊ����ģʽ��2Ϊ����ʱģʽ��3Ϊ���÷�ģʽ��4Ϊ������ģʽ

unsigned char displayIndex = 0;
unsigned char LED8[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned int interruptCount = 0; // �жϴ���

sbit SEG_DS = P2 ^ 0;   // 74HC595оƬ����������
sbit SEG_SHCP = P2 ^ 1; // 74HC595оƬ�Ŀ������ţ���������������
sbit SEG_STCP = P2 ^ 2; // 74HC595оƬ�Ŀ������ţ������ظ�������

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
    0xFF, /*OFFF*/
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

void DisplayOneCharOnAddr(unsigned char, unsigned char Addr);
void SEG_Send595OneByte(unsigned char ucData); // ��74HC595д��һ��8λ������
void SecondIncrease();

unsigned int checkCount = 0;
unsigned char shortOrLang = 0; // 0��ʾ��Ч��1��ʾ�̰���2��ʾ����

bit buttonDown = 0;
bit button = 0;

sbit P3_2 = P3 ^ 2;
sbit P3_3 = P3 ^ 3;

void Check() // �жϰ����ǳ������Ƕ̰���
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

void ShortPress() // �̰��������
{
    if (button == 0) // ��������Ǽ�A
    {
        switch (mode)
        {
        case SHOW:
            mode = SET;
            break;
        case SET:
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
        unsigned char tmp_hour = LED8[0] * 10 + LED8[1];
        unsigned char tmp_minute = LED8[3] * 10 + LED8[4];
        unsigned char tmp_second = LED8[6] * 10 + LED8[7];
        switch (mode)
        {
        case SET_HOUR:
            tmp_hour++;
            tmp_hour %= 24;
            LED8[0] = tmp_hour / 10;
            LED8[1] = tmp_hour % 10;
            break;
        case SET_MINUTE:
            tmp_minute++;
            tmp_minute %= 60;
            LED8[3] = tmp_minute / 10;
            LED8[4] = tmp_minute % 10;
            break;
        case SET_SECOND:
            tmp_second++;
            tmp_second %= 60;
            LED8[6] = tmp_second / 10;
            LED8[7] = tmp_second % 10;
            break;

        default:
            break;
        }
    }

    shortOrLang = 0;
}

void LongPress() // �����������
{
    if (button == 0) // ��������Ǽ�A
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
        case SET_HOUR:
        case SET_MINUTE:
        case SET_SECOND:
            mode = SHOW;
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

void Display(unsigned char enable)
{
    enable >>= displayIndex;
    enable &= 1;
    if (enable == 1)
        DisplayOneCharOnAddr(LED8[displayIndex], displayIndex); // ��ʾ�����
    else
        DisplayOneCharOnAddr(17, displayIndex);
    displayIndex++;
    displayIndex %= 8; // ��ʾ��һ�������
}

void Timer0() interrupt 1
{ // ��ʱ��0�жϷ�����
    interruptCount++;

    TH0 = (65536 - INTERVAL * 1000) / 256;
    TL0 = (65536 - INTERVAL * 1000) % 256;

    if (interruptCount == 1000 / INTERVAL)
    { // 1��
        interruptCount = 0;
        SecondIncrease();
    }

    if (buttonDown == 1)
    {
        Check();
    }

    if (shortOrLang != 0)
    {
        if (shortOrLang == 1)
        {
            ShortPress();
        }
        else
        {
            LongPress();
        }
    }

    switch (mode)
    {
    case SHOW:
        LED8[0] = hour / 10; // ��ʾСʱʮλ
        LED8[1] = hour % 10; // ��ʾСʱ��λ
        LED8[2] = 16;
        LED8[3] = minute / 10; // ��ʾ����ʮλ
        LED8[4] = minute % 10; // ��ʾ���Ӹ�λ
        LED8[5] = 16;
        LED8[6] = second / 10; // ��ʾ��ʮλ
        LED8[7] = second % 10; // ��ʾ���λ
        Display(0xFF);
        break;
    case SET:
        if (interruptCount < 500)
            Display(0xFF);
        else
            Display(0); // �ر������
        break;
    case SET_HOUR:
        if (interruptCount < 500)
            Display(0xFF);
        else
            Display(0xFC);
        break;
    case SET_MINUTE:
        if (interruptCount < 500)
            Display(0xFF);
        else
            Display(0xE7);
        break;
    case SET_SECOND:
        if (interruptCount < 500)
            Display(0xFF);
        else
            Display(0x3F);
        break;
    default:
        break;
    }
}

void Init()
{
    mode = SHOW;

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

/*******************************************************
�������ܣ���ָ��λ����ʾһ������
����˵����Data��Ҫ��ʾ�����ݣ�Addr���ڵڼ�λ��ʾ��

Addrȡֵ��Χ��0~9��
Addr=0~7ʱ��ѡ�������ʾ�ڵڼ�λ�������;
Addr=8  ʱ��ͬʱѡ��8λ����ܣ��������������
Addr=9  ʱ���ر�8λ�����

8λ����ܣ���������Ϊ��0λ����1λ...��7λ��

*******************************************************/
void DisplayOneCharOnAddr(unsigned char Data, unsigned char Addr)
{
    SEG_Send595OneByte(Seg_Addr[Addr]); // ��ʾ����һ���������
    SEG_Send595OneByte(Seg_Data[Data]); // ��ʾ������
    SEG_STCP = 0;
    SEG_STCP = 1; // STCP���ŵ������ظ�������
    SEG_STCP = 0;
}
