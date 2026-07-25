/*
 * ������������Ӳ�������������չ����Ӳ�����Ϲ���ȫ����Դ
 * �����������www.lckfb.com
 * ����֧�ֳ�פ��̳���κμ������⻶ӭ��ʱ����ѧϰ
 * ������̳��https://oshwhub.com/forum
 * ��עbilibili�˺ţ������������塿���������ǵ����¶�̬��
 * ��������׬Ǯ���������й�����ʦΪ����
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-30     LCKFB        ������
 */

#include "bsp_gyro.h"
#include "stdio.h"
#include "string.h"
#include "uart.h" 


volatile Gyro_Struct Gyro_Structure;
float RollX,PitchY ,YawZ;
float WX,WY ,WZ;


void jy61pInit(void)
{

	/*================Z�����==================*/

	// �Ĵ�������
	uint8_t unlock_reg1[2] = {0x88,0xB5};
	writeDataJy61p(IIC_ADDR,UN_REG,unlock_reg1,2);
	delay_ms(200);
	// Z�����
	uint8_t z_axis_reg[2] = {0x04,0x00};
	writeDataJy61p(IIC_ADDR,ANGLE_REFER_REG,z_axis_reg,2);
	delay_ms(200);
	// ����
	uint8_t save_reg1[2] = {0x00,0x00};
	writeDataJy61p(IIC_ADDR,SAVE_REG,save_reg1,2);
	delay_ms(200);

	/*================�Ƕȹ���==================*/

	// �Ĵ�������
	uint8_t unlock_reg[2] = {0x88,0xB5};
	writeDataJy61p(IIC_ADDR,UN_REG,unlock_reg,2);
	delay_ms(200);
	// �Ƕȹ���
	uint8_t angle_reg[2] = {0x08,0x00};
	writeDataJy61p(IIC_ADDR,ANGLE_REFER_REG,angle_reg,2);
	delay_ms(200);
	// ����
	uint8_t save_reg[2] = {0x00,0x00};
	writeDataJy61p(IIC_ADDR,SAVE_REG,save_reg,2);
	delay_ms(200);

	// ��սṹ��
	memset((void*)&Gyro_Structure,0,sizeof(Gyro_Structure));
}



/******************************************************************
 * �� �� �� �ƣ�IIC_Start
 * �� �� ˵ ����IIC��ʼʱ��
 * �� �� �� �Σ���
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Start(void)
{
        SDA_OUT();

        SCL(0);
        SDA(1);
        SCL(1);

        delay_us(5);

        SDA(0);
        delay_us(5);
        SCL(0);
        delay_us(5);


}
/******************************************************************
 * �� �� �� �ƣ�IIC_Stop
 * �� �� ˵ ����IICֹͣ�ź�
 * �� �� �� �Σ���
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Stop(void)
{
        SDA_OUT();
        SCL(0);
        SDA(0);

        SCL(1);
        delay_us(5);
        SDA(1);
        delay_us(5);

}

/******************************************************************
 * �� �� �� �ƣ�IIC_Send_Ack
 * �� �� ˵ ������������Ӧ����߷�Ӧ���ź�
 * �� �� �� �Σ�0����Ӧ��  1���ͷ�Ӧ��
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Send_Ack(unsigned char ack)
{
        SDA_OUT();
        SCL(0);
        SDA(0);
        delay_us(5);
        if(!ack) SDA(0);
        else     SDA(1);
        SCL(1);
        delay_us(5);
        SCL(0);
        SDA(1);
}


/******************************************************************
 * �� �� �� �ƣ�I2C_WaitAck_gyro
 * �� �� ˵ �����ȴ��ӻ�Ӧ��
 * �� �� �� �Σ���
 * �� �� �� �أ�0��Ӧ��  1��ʱ��Ӧ��
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned char I2C_WaitAck_gyro(void)
{

        char ack = 0;
        char ack_flag = 50;

        SDA_IN();

        SDA(1);
        while( (SDA_GET()==1) && ( ack_flag ) )
        {
                ack_flag--;
                delay_us(5);
        }

        if( ack_flag == 0 )
        {
                IIC_Stop();
                return 1;
        }
        else
        {
				SCL(1);
				delay_us(5);
                SCL(0);
                SDA_OUT();
        }
        return ack;
}

/******************************************************************
 * �� �� �� �ƣ�Send_Byte_gyro
 * �� �� ˵ ����д��һ���ֽ�
 * �� �� �� �Σ�datҪд�˵�����
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void Send_Byte_gyro(uint8_t dat)
{
	int i = 0;
	SDA_OUT();
	SCL(0);//����ʱ�ӿ�ʼ���ݴ���

	for( i = 0; i < 8; i++ )
	{
		SDA( (dat & 0x80) >> 7 );
		delay_us(2);

		SCL(1);
		delay_us(5);

		SCL(0);
		delay_us(5);

		dat<<=1;
	}
}

/******************************************************************
 * �� �� �� �ƣ�Read_Byte
 * �� �� ˵ ����IIC��ʱ��
 * �� �� �� �Σ���
 * �� �� �� �أ�����������
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned char Read_Byte(void)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
    {
        SCL(0);
        delay_us(5);
        SCL(1);
        delay_us(5);
        receive<<=1;
        if( SDA_GET() )
        {
            receive|=1;
        }
        delay_us(5);
    }

    return receive;
}

/******************************************************************
 * �� �� �� �ƣ�writeDataJy61p
 * �� �� ˵ ����д����
 * �� �� �� �Σ�dev �豸��ַ
				reg �Ĵ�����ַ
				data �����׵�ַ
				length ���ݳ���
 * �� �� �� �أ�����0��д��ɹ�
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
uint8_t writeDataJy61p(uint8_t dev, uint8_t reg, uint8_t* data, uint32_t length)
{
    uint32_t count = 0;

    IIC_Start();

    Send_Byte_gyro(dev<<1);
    if(I2C_WaitAck_gyro() == 1)return 0;

    Send_Byte_gyro(reg);
    if(I2C_WaitAck_gyro() == 1)return 0;

    for(count=0; count<length; count++)
    {
        Send_Byte_gyro(data[count]);
        if(I2C_WaitAck_gyro() == 1)return 0;
    }

    IIC_Stop();

    return 1;
}

/******************************************************************
 * �� �� �� �ƣ�readDataJy61p
 * �� �� ˵ ��������������
 * �� �� �� �Σ�dev �豸��ַ
				reg �Ĵ�����ַ
				data ���ݴ洢��ַ
				length ���ݳ���
 * �� �� �� �أ�����0��д��ɹ�
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
uint8_t readDataJy61p(uint8_t dev, uint8_t reg, uint8_t *data, uint32_t length)
{
    uint32_t count = 0;

    IIC_Start();

    Send_Byte_gyro((dev<<1)|0);
    if(I2C_WaitAck_gyro() == 1)return 0;

    Send_Byte_gyro(reg);
    if(I2C_WaitAck_gyro() == 1)return 0;

	delay_us(5);

    IIC_Start();

    Send_Byte_gyro((dev<<1)|1);
    if(I2C_WaitAck_gyro() == 1)return 0;

    for(count=0; count<length; count++)
    {
        if(count!=length-1)
		{
			data[count]=Read_Byte();
			IIC_Send_Ack(0);
		}
        else
		{
			data[count]=Read_Byte();
			IIC_Send_Ack(1);
		}

    }

    IIC_Stop();

    return 1;
}

/******************************************************************
 * �� �� �� �ƣ�get_angle
 * �� �� ˵ �������Ƕ�����
 * �� �� �� �Σ���
 * �� �� �� �أ����ؽṹ��
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void get_angle(void)
{
	// ���ݻ���
	uint8_t sda_angle[6] = {0};

	int ret = 0;

	// ������ݻ���
	memset(sda_angle,0,sizeof(sda_angle));

	// ��ȡ�Ĵ�������
	ret	= readDataJy61p(IIC_ADDR,0x3D,sda_angle,6);

	if(ret == 0)
	{
		// ��ȡʧ��
		printf("Read Error\r\n");
	}

	#if GYRO_DEBUG

	printf("RollL = %x\r\n",sda_angle[0]);
	printf("RollH = %x\r\n",sda_angle[1]);
	printf("PitchL = %x\r\n",sda_angle[2]);
	printf("PitchH = %x\r\n",sda_angle[3]);
	printf("YawL = %x\r\n",sda_angle[4]);
	printf("YawH = %x\r\n",sda_angle[5]);

	#endif

    // ���� RollX, PitchY �� YawZ ��ȷ�������� -180 �� 180 �ķ�Χ��
    RollX = (float)(((sda_angle[1] << 8) | sda_angle[0]) / 32768.0 * 180.0);
    if (RollX > 180.0)
    {
        RollX -= 360.0;
    }
    else if (RollX < -180.0)
    {
        RollX += 360.0;
    }

    PitchY = (float)(((sda_angle[3] << 8) | sda_angle[2]) / 32768.0 * 180.0);
    if (PitchY > 180.0)
    {
        PitchY -= 360.0;
    }
    else if (PitchY < -180.0)
    {
        PitchY += 360.0;
    }

    YawZ = (float)(((sda_angle[5] << 8) | sda_angle[4]) / 32768.0 * 180.0);
    if (YawZ > 180.0)
    {
        YawZ -= 360.0;
    }
    else if (YawZ < -180.0)
    {
        YawZ += 360.0;
    }

    // �����������浽�ṹ����
    Gyro_Structure.x = RollX;
    Gyro_Structure.y = PitchY;
    Gyro_Structure.z = YawZ;

}


void get_wx_wy_wz(void)
{
	// ���ݻ���
	uint8_t sda_x[6] = {0};
	int ret = 0;
	// ������ݻ���
	memset(sda_x,0,sizeof(sda_x));
	// ��ȡ�Ĵ�������
	ret	= readDataJy61p(IIC_ADDR,0x37,sda_x,6);
	if(ret == 0)
	{
		// ��ȡʧ��
		printf("Read Error\r\n");
	}
    // ����
    WX = (float)(((sda_x[1] << 8) | sda_x[0]) / 32768.0 * 2000.0);
	if(WX>2000)			WX-=4000;
    WY = (float)(((sda_x[3] << 8) | sda_x[2]) / 32768.0 * 2000.0);
	if(WY>2000)			WY-=4000;		
    WZ = (float)(((sda_x[5] << 8) | sda_x[4]) / 32768.0 * 2000.0);
	if(WZ>2000)			WZ-=4000;
	
}


void get_jy61p_data(void)
{	
	get_angle();
	get_wx_wy_wz();
}

