/*
    ******************************************************************************
    * @Company              : Woojin Industrial Systems Co., Ltd.
    * @Engineer             : D.S.JANG
    * @ModuleName           : LED.c
    * @ProjectName          : SEPTA ML
    * @TargetDevices        : LEDC-A5.2
    * @ToolVersions         : IAR 8.32.2.
    * Dependencies          : SEPTA_ML_FDI.BIN
    * @Revision             : 1.0.0.0
    ******************************************************************************
    * @AdditionalComments   :
    *                        1) Ver 1.0.0.0 : 2018/07/11 First release.
    ******************************************************************************
*/



#include "stm32f4xx_hal.h"

#include "test.h"
#include "../Utilities/Fonts/fonts.h"
#include "main.h"
#include "I2C.h"
#include "AmpGpio.h"
#include "stdbool.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;



/*****************************************************************************
* @brief -
* @param -
* @retval-
******************************************************************************/
void MX_I2C1_Init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	GPIO_InitStructure.Pin =  GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;
        
	__GPIOB_CLK_ENABLE();
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
        

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        
        

	/* USER CODE BEGIN I2C2_Init 0 */

	/* USER CODE END I2C2_Init 0 */

	/* USER CODE BEGIN I2C2_Init 1 */

	/* USER CODE END I2C2_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
      
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}


}


/*****************************************************************************
* @brief -
* @param -
* @retval-
******************************************************************************/
void MX_I2C2_Init(void)
{
  
	GPIO_InitTypeDef   GPIO_InitStructure;

	GPIO_InitStructure.Pin =  GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C2;
        
	__GPIOB_CLK_ENABLE();
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
        

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);

	/* USER CODE BEGIN I2C2_Init 0 */

	/* USER CODE END I2C2_Init 0 */

	/* USER CODE BEGIN I2C2_Init 1 */

	/* USER CODE END I2C2_Init 1 */
	hi2c2.Instance = I2C2;
	hi2c2.Init.ClockSpeed = 400000;
	hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
      
	if (HAL_I2C_Init(&hi2c2) != HAL_OK)
	{
		Error_Handler();
	}

}


//--------------------------------------------------------------------------------------------//
// 전류 센서 측정 하는 부분.
// 최종 기능 업데이트시 적용 필요.   ( LED가 동작무 부하일때 측정중지하는 부분 필요 -  무부하일때 측정값 틀어짐 확인)
//--------------------------------------------------------------------------------------------//
// I2C_HAL_ReadBytes
// I2C_HAL_WriteBytes
void processCurrentVal(void)
{
       uint8_t     nRbuf[10];
       int sData_val = 0;
       
    static int bCurLedCtr =1;


    if(bCurLedCtr) // 전류 센서 값은 LED 동작 중일 때만 동작 한다.
    {
        nRbuf[0] = 0x41;
        nRbuf[1] = 0x27;   
            
        if(I2C_HAL_WriteBytes(&hi2c2,0x80,0x00,(uint8_t *)nRbuf,2))
        {
            MyPrintf_USART1("++++++++++++ Write Calibration Register  ++++++++++++ \n\r" );
        }
        else
        {
            MyPrintf_USART1( "++Write Test NG \n\r" );
        }


        nRbuf[0] = 0xFF;
        nRbuf[1] = 0xFF;  
         if(I2C_HAL_ReadBytes(&hi2c2,0x80,0x01,(uint8_t *)nRbuf,2))
        {
            //printf("++ INA226 OK \n\r");
                
             MyPrintf_USART1( "++ Read Shunt Register (%02X,%02X) ------\r\n",nRbuf[0],nRbuf[1] );
        }
        else
        {
            MyPrintf_USART1( "++ INA226 NG \n\r" );
        }


            
         if(I2C_HAL_ReadBytes(&hi2c2,0x80,0x02,(uint8_t *)nRbuf,2))
        {
            //printf("++ INA226 OK \n\r");
             //소주점 2번째 가지까지 환산. (ex : 2427 -> 24.27v)
             sData_val = (float)((nRbuf[0] << 8) | nRbuf[1]) / 7.77; 
                
             MyPrintf_USART1( "++ Read Bus Voltage Register (%d) \r\n",sData_val );
        }
        else
        {
            MyPrintf_USART1( "++ INA226 NG \n\r" );
        }
            
                
                    
        /////////////////////////////////////IC  상태를 전류 측정 상태로 변경 하는 값.        
            nRbuf[0] = 0x0A;
            nRbuf[1] = 0x00;   
                
            if(I2C_HAL_WriteBytes(&hi2c2,0x80,0x05,(uint8_t *)nRbuf,2))
            {
                MyPrintf_USART1("++ Write V-C Change \r\n" );
            }
            else
            {
                MyPrintf_USART1( "++Write Calibration NG \n\r" );
            }
      /////////////////////////////////////                  
                
         if(I2C_HAL_ReadBytes(&hi2c2,0x80,0x04,(uint8_t *)nRbuf,2))
        {
           
             // mA로 표현 --> 초기 대구 
             sData_val = (float)((nRbuf[0] << 8) | nRbuf[1]) / 2.6; 
             
             if(mLed_Process_Flag.sCurrentVal < sData_val) // 1초에 한번씩 체크하고, 최대 값을 저장 하도록 수정.
             {
                mLed_Process_Flag.sCurrentVal = sData_val; // 전류 측정 값 리턴.  
             }
            
            MyPrintf_USART1( "++ Read Current Register(%03d) \n\r",sData_val );
        }
        else
        {
            MyPrintf_USART1( "++ INA226 Read NG \n\r");
        }
            
                
                    
                
        if(I2C_HAL_ReadBytes(&hi2c2,0x80,0x03,(uint8_t *)nRbuf,2))
        {
            //printf("++ INA226 OK \n\r");
                
             MyPrintf_USART1( "++ Read Power Register(%02X,%02X) \n\r",nRbuf[0],nRbuf[1] );
        }
        else
        {
            MyPrintf_USART1( "++ INA226 NG \n\r" );
                
        }
                    
    }

}


/*****************************************************************************
* @brief -
* @param -
* @retval-
******************************************************************************/
//========================================================================
int I2C_HAL_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t TxBufferSize)
//========================================================================
{
	int TimeOut;

	TimeOut = 0;
	while (HAL_I2C_Mem_Write(hi2c, (uint16_t)DevAddress, (uint16_t)MemAddress, I2C_MEMADD_SIZE_8BIT, pData, (uint16_t)TxBufferSize,2) != HAL_OK && TimeOut < 2) TimeOut++;

	HAL_Delay(5);

	return 1;
}


/*****************************************************************************
* @brief -
* @param -
* @retval-
******************************************************************************/
//========================================================================
int I2C_HAL_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t RxBufferSize)
//========================================================================
{
	int TimeOut;

	TimeOut = 0;
	while (HAL_I2C_Mem_Read(hi2c, (uint16_t)DevAddress, (uint16_t)MemAddress, I2C_MEMADD_SIZE_8BIT, pData, (uint16_t)RxBufferSize, 2) != HAL_OK && TimeOut < 2) TimeOut++;

	return 1;
}





