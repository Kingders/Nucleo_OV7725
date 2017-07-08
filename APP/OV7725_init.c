
#include "stm32f7xx_hal.h"
#include "OV7725_init.h"
#include "Transmission_mode.h"

extern uint8_t pData[NR][NC];
uint8_t rdata[1] = {0};
HAL_StatusTypeDef state;
//Loop to initiate the camera register
void DCMI_OV7725_QVGASizeRaWSetup(I2C_HandleTypeDef *hi2c1)
{
	int i;
	const  char change_reg[74][3]=
	{
		//test
		{0x12,0x80},
		{0x08,0x00},
		{0x10,0x00},

		//test
		{0x32,0x00},  // HREF
		{0x2a,0x00},  // EXHCH  Dummy Pixel Insert MSB
		{0x11,0x01},	// CLKRC
		{0x12,0x41},  // QVGA Processed Bayer RAW		   // COM7
									// {0x12,0x53}, //QVGA Sensor Raw, Bayer RAW
									// {0x12,0x46}, //QVGA RGB565
									// {0x12,0x06}, //VGA  RGB565
		{0x42,0x7f},  // BLC Blue Channel Target Value 
		{0x4d,0x00},  
		{0x63,0xf0},
		{0x64,0xff},  // DSP Control Byte 1
		{0x65,0x20},	// DSP Control Byte 2
		{0x66,0x00},  // DSP Control Byte 3
		{0x67,0x02},  // RAW8
		//{0x67,0x03},  // RAW10
		{0x69,0x5d},
		{0x13,0xfe},	// ACE Auto Disable AWB Auto Disable
		//{0x13,0xff},	// ACE Auto Disable AWB Auto Disable
		{0x0d,0x40},  //PLL
		{0x0f,0xc5},
		{0x14,0x11},
		{0x22,0xFF},  //7f
		{0x23,0x01},
		{0x24,0x34},
		{0x25,0x3c},
		{0x26,0xa1},
		{0x2b,0x00},
		{0x6b,0xaa},
		{0x90,0x0a}, 
		{0x91,0x01}, 
		{0x92,0x01}, 
		{0x93,0x01},
		{0x94,0x5f},
		{0x95,0x53},
		{0x96,0x11},
		{0x97,0x1a},
		{0x98,0x3d},
		{0x99,0x5a},
		{0x9a,0x1e},
		{0x9b,0x0 },//set luma 
		{0x9c,0x25},//set contrast 
		{0xa7,0x65},//set saturation 
		{0xa8,0x65},//set saturation 
		{0xa9,0x80},//set hue 
		{0xaa,0x80},//set hue 
		{0x9e,0x81},
		{0xa6,0x06},
		{0x7e,0x0c},
		{0x7f,0x16},
		{0x80,0x2a},
		{0x81,0x4e},
		{0x82,0x61},
		{0x83,0x6f},
		{0x84,0x7b},
		{0x85,0x86},
		{0x86,0x8e},
		{0x87,0x97},
		{0x88,0xa4},
		{0x89,0xaf},
		{0x8a,0xc5},
		{0x8b,0xd7},
		{0x8c,0xe8},
		{0x8d,0x20},
		{0x33,0x00},
		{0x22,0x99},
		{0x23,0x03},
		{0x4a,0x00},
		{0x49,0x13},
		{0x47,0x08},
		{0x4b,0x14},
		{0x4c,0x17},
		{0x46,0x05},
		{0x0e,0xf5}, 
		{0x0c,0xd0}, //YUV, Vertical flip image ON, Horizontal mirror image ON
		//{0x08,0xFF},
		{0x10,0x19}
	};
//	const uint8_t change_reg[11][2] = {
//			{0x12,0x80},
//			{0x11,0x01},
//			{0x0d,0x40},
//			{0x12,0x53},
//			{0x2a,0x00},
//			{0x2b,0x00},
//			{0x33,0x00},
//			{0x0e,0xf5}};
	while(HAL_I2C_GetState(hi2c1) != HAL_I2C_STATE_READY);
	HAL_I2C_Master_Transmit(hi2c1, 0x42, (uint8_t*)&change_reg[0][0], (uint16_t)2, 1000);
	HAL_Delay(1);
	
	for(i = 1; i < 74; i++)
	{
		while(HAL_I2C_GetState(hi2c1) != HAL_I2C_STATE_READY);
		HAL_I2C_Master_Transmit(hi2c1, 0x42, (uint8_t*)&change_reg[i][0], (uint16_t)2, 1000);
		HAL_Delay(10);
		while(1)
		{
			while(HAL_I2C_GetState(hi2c1) != HAL_I2C_STATE_READY);
			HAL_I2C_Master_Transmit(hi2c1, 0x43, (uint8_t*)&change_reg[i][0],1,1000);
			HAL_Delay(10);
			while(HAL_I2C_GetState(hi2c1) != HAL_I2C_STATE_READY);
			HAL_I2C_Master_Receive(hi2c1, 0x43, rdata,1,1000);
			if(rdata[0] == change_reg[i][1])
				break;
			else
			{
				while(HAL_I2C_GetState(hi2c1) != HAL_I2C_STATE_READY);
				state = HAL_I2C_Master_Transmit(hi2c1, 0x42, (uint8_t*)&change_reg[i][0], (uint16_t)2, 1000);
				if(state != HAL_OK)
				{
					HAL_I2C_DeInit(hi2c1);
					HAL_Delay(1);
					HAL_I2C_Init(hi2c1);
					HAL_Delay(10);
				}
				HAL_Delay(1);
			}
		}
	}
	HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_SET);
}

//funtion to interpolaze the bayer pattern to color image
void bayer2rgb(unsigned char *CAMBuffer)
{
		int i,j,R,G,B, index;
		int nr = NR, nc = NC;

		for(i=0;i<nr;i++)
		{
			for(j=0;j<nc;j++)
			{
				if(i%2==1 && j%2==0)
				{
					R=(pData[i-1][j-1]+pData[i-1][j+1]+pData[i+1][j-1]+pData[i+1][j+1])/4;
					G=(pData[i][j-1]+pData[i][j+1]+pData[i-1][j]+pData[i+1][j])/4;
					B=pData[i][j];
				}
				else if(i%2==1 && j%2==1)
				{
					R=(pData[i-1][j]+pData[i+1][j])/2;
					G=pData[i][j];
					B=(pData[i][j-1]+pData[i][j+1])/2;
				}
				else if(i%2==0 && j%2==0)
				{
					R=(pData[i][j-1]+pData[i][j+1])/2;
					G=pData[i][j];
					B=(pData[i-1][j]+pData[i+1][j])/2;
				} 
				else if(i%2==0 && j%2==1)
				{
					R=pData[i][j];
					G=(pData[i][j-1]+pData[i][j+1]+pData[i-1][j]+pData[i+1][j])/4;
					B=(pData[i-1][j-1]+pData[i-1][j+1]+pData[i+1][j-1]+pData[i+1][j+1])/4;
				}

				#if IMAGE_TYPE == GRAY_IMAGE
					CAMBuffer[(i*nc + j)]=(R+G+B)/3;
				#elif IMAGE_TYPE == COLOR_IMAGE
					index = (i * nc + j) * 3;
					#if SEND_MODE == USB_SEND_IMAGE
						CAMBuffer[index]=B;
						CAMBuffer[index+1]=G;
						CAMBuffer[index+2]=R;				
					#elif SEND_MODE == UART_SEND_IMAGE
						CAMBuffer[index]=R;
						CAMBuffer[index+1]=G;
						CAMBuffer[index+2]=B;
					#endif
				#endif
			}
		}
		
	
}


