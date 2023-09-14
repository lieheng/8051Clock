#include <REG51.H>

#define INTERVAL 1 // ���1����

#define INIT_HOUR 23   // ��ʼ��Сʱ
#define INIT_MINUTE 59 // ��ʼ������
#define INIT_SECOND 20 // ��ʼ����

unsigned char hour = INIT_HOUR;     // ��ʼ��Сʱ
unsigned char minute = INIT_MINUTE; // ��ʼ������
unsigned char second = INIT_SECOND; // ��ʼ����

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
    DisplayOneCharOnAddr(LED8[displayIndex], displayIndex); // ��ʾ�����
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

    LED8[0] = hour / 10; // ��ʾСʱʮλ
    LED8[1] = hour % 10; // ��ʾСʱ��λ
    LED8[2] = 16;
    LED8[3] = minute / 10; // ��ʾ����ʮλ
    LED8[4] = minute % 10; // ��ʾ���Ӹ�λ
    LED8[5] = 16;
    LED8[6] = second / 10; // ��ʾ��ʮλ
    LED8[7] = second % 10; // ��ʾ���λ

    Display();
}

void Init()
{
    EA = 1; // �������ж�

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
