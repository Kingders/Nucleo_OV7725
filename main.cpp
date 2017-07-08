#include "mbed.h"
extern "C"{
#include "MIAT_System.h"
#include "usb_device.h"
#include "OV7725_init.h"
#include "Transmission_mode.h"
#include "jpeg_utils.h"
#include "encode_dma.h"
}

Serial pc(SERIAL_TX, SERIAL_RX);  // PD8,PD9

extern "C"{
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

//UART_HandleTypeDef huart2;
//UART_HandleTypeDef huart3;

Serial uart2(PD_5,PD_6);
	
void Error_Handler(void);
static void MX_TIM3_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
//static void MX_USART2_UART_Init(void);

uint8_t pData[NR][NC*2] = {0};

#if IMAGE_TYPE == GRAY_IMAGE
	uint8_t gray_image[NUM_OF_PIXELS] = {0};
	uint8_t format[7] = {"GSCALE"};
#elif IMAGE_TYPE == COLOR_IMAGE
	uint8_t color_image[NUM_OF_COLOR_PIXELS] = {0};
	uint8_t format[7] = {"RGB888"};
#endif

uint8_t buf[1] = {0};
//uint8_t width[2] = {0};
//uint8_t height[2] = {0};
uint8_t dcmi_flag = 0;
//const int TIMEOUT = 5000;
//const int PACKET_SIZE = 65535;

volatile uint8_t isReady = 0;
volatile uint8_t isRead = 1;

volatile uint8_t bGetReady = FALSE;
JPEG_HandleTypeDef    JPEG_Handle;
}

#if 0
static void UART_SendQuery()
{
	uint8_t format[6] = {"QUERY"};
	
	HAL_UART_Transmit(&huart2, format, (uint16_t) 5, TIMEOUT);
}

static void UART_SendFinish()
{
	uint8_t format[7] = {"FINISH"};
	
	HAL_UART_Transmit(&huart2, format, (uint16_t) 6, TIMEOUT);
}

static void UART_SendStart()
{
	uart2.putc(0xA6);
	
	DEBUG("\rUART_SendStart\n");
}
#endif

static void UART_GetReady()
{
	#if 0
	buf[0]=0;

	if(HAL_UART_Receive(&huart2, &buf[0], (uint16_t) 1, 200) != HAL_OK)
		ERROR("\rERROR: HAL_UART_Receive\n");
	
	if(buf[0] == 0xA5)
		return 0;
	else
	{
		if(buf[0]!=0)
			printf("\r0x%x\n",buf[0]);
		
		return 1;
	}
#else
/*
	if(uart2.getc() == 0xA5)
		return 0;
	else
		return 1;
*/
	if(uart2.getc() == 0xA5)
	{
		bGetReady = TRUE;
	
		DEBUG("\rUART_GetReady\n");
		
		//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)pData, NR * NC);
	}
	else
		bGetReady = FALSE;
#endif
}

void UART_SendData(uint8_t* pbuf, uint16_t size)
{
	DEBUG("\rUART_SendData IN\n");
	
	uint16_t i;
					
	for(i=0; i<size; i++)
	{
		uart2.putc(*(pbuf+i));
	}
	DEBUG("\rUART_SendData OUT\n");
}

void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi){
  /* Prevent unused argument(s) compilation warning */
	uint32_t JpegEncodeProcessing_End = 0;
	
	DEBUG("\rHAL_DCMI_VsyncEventCallback\n");
	
//	if(bGetReady == FALSE)
//		return;
					
	dcmi_flag++;
	if(dcmi_flag == 2){
		dcmi_flag = 0;
		HAL_DCMI_Stop(hdcmi);
		
		#if IMAGE_TYPE == GRAY_IMAGE
			bayer2rgb(gray_image);
		#elif IMAGE_TYPE == COLOR_IMAGE
			bayer2rgb(color_image);
		#endif
		
		//start image operation.....
    /*##	JPEG Encoding with DMA (Not Blocking ) Method ################*/


		DEBUG("\rJPEG_Encode_DMA\n");
			
		JPEG_Encode_DMA(&JPEG_Handle);
    		
		/*##	Wait till end of JPEG encoding and perfom Input/Output Processing in BackGround  #*/
		do
		{
			JPEG_EncodeInputHandler(&JPEG_Handle);
			JpegEncodeProcessing_End = JPEG_EncodeOutputHandler(&JPEG_Handle);
            
		}while(JpegEncodeProcessing_End == 0);		

		
		//end image operation.....
			bGetReady = FALSE;	
			HAL_DCMI_Start_DMA(hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)pData, NR * NC);
		
			//configure the picture resolution
/*
			width[0] = NC & 0xff;
			width[1] = (NC >> 8) & 0xff;
			height[0] = NR & 0xff;
			height[1] = (NR >> 8) & 0xff;


			//send the resolution data to the MIAT software
			//format + width + height + image data
			int i;
			HAL_UART_Transmit(&huart2, format, (uint16_t) 6, TIMEOUT);
			for(i=1;i>=0;i--) HAL_UART_Transmit(&huart2, &width[i], (uint16_t) 1, TIMEOUT);
			for(i=1;i>=0;i--) HAL_UART_Transmit(&huart2, &height[i], (uint16_t) 1, TIMEOUT);
*/

	}

}

HAL_StatusTypeDef stateDCMI = HAL_OK;
int main()
{
	HAL_Init();
	SystemClock_Config();
	pc.baud(9600);
	
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USB_DEVICE_Init();
	
	DEBUG("\rMX_DCMI_Init\n");
	MX_DCMI_Init();
	DEBUG("\rMX_I2C1_Init\n");
	MX_I2C1_Init();
	DEBUG("\rMX_TIM3_Init\n");
	MX_TIM3_Init();
	HAL_TIM_Base_Start(&htim3); 
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	HAL_Delay(1000);
	DEBUG("\rDCMI_OV7725_QVGASizeRaWSetup\n");
	DCMI_OV7725_QVGASizeRaWSetup(&hi2c1);
	HAL_Delay(1100);

  /* Init The JPEG Color Look Up Tables used for YCbCr to RGB conversion   */ 
  JPEG_InitColorTables();
	
   /* Init the HAL JPEG driver */
  JPEG_Handle.Instance = JPEG;
	
  if(HAL_JPEG_Init(&JPEG_Handle)!=HAL_OK)
			ERROR("\rHAL_JPEG_Init ERROR\n");
	
	if(stateDCMI == HAL_OK)
	{
		DEBUG("\rUSART2 init\n");
		
		//MX_USART2_UART_Init();
		//uart2.baud(115200);
		uart2.baud(921600);
		uart2.format(8, SerialBase::None, 1);
		uart2.attach(&UART_GetReady);
	}
	else
		DEBUG("\r[ERROR]HAL_DCMI_Start_DMA\n");
	
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)pData, NR * NC);
	
	while (true) 
	{
/*
		if(isRead == 1)
		{
			isRead = 0;
			isReady = 0;
			
			//DEBUG("\rHAL_DCMI_Start_DMA\n");
			
			//stateDCMI = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)pData, NR * NC);
		}
*/		
	}	
		
}

static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
   
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    
  }

  HAL_TIM_MspPostInit(&htim3);

}


static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}


static void MX_DCMI_Init(void)
{

  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }

}





static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x70102AFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

}

static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin : User_Blue_Button_Pin */
  GPIO_InitStruct.Pin = User_Blue_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(User_Blue_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

}

#if 0
/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}
#endif

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}
