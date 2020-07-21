/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SSD1331.h"
#include "audioI2S.h"
#include "MY_CS43L22.h"
#include "wav_player.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern ApplicationTypeDef Appli_state;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

float potencjometr;
float potencjometr_255;
int potencjometr_255_int=100;

int volume = 7;
int old_volume = 0;
int actually_track = 0;
int last_track=0;
int seconds = 0;
int tens_of_seconds = 0;
int minutes = 0;
char time[10];
char time_clear[10];

bool is_playing=false;
const char* tracks[3];


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	 potencjometr = HAL_ADC_GetValue(&hadc2);
	 potencjometr_255 = (potencjometr/4300.0)*255;
	 potencjometr_255_int = potencjometr_255;
	 volume = potencjometr_255 / 24;
	 HAL_ADC_Start_IT(&hadc2);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
 if(htim->Instance == TIM2){
 	 HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
 	 if(is_playing)
 	 {
 	ssd1331_display_string(28, 27, time, FONT_1608, BLACK);
 	 if (seconds == 10)
 	        {
 	            seconds = 0;
 	            tens_of_seconds++;
 	        }
 	        if (tens_of_seconds == 6)
 	        {
 	            tens_of_seconds = 0;
 	            minutes++;
 	        }
 	        sprintf(time,"%d:%d%d", minutes, tens_of_seconds, seconds);
 	        ssd1331_display_string(28, 27, time, FONT_1608, BLUE);
 	        seconds++;
 	 }

 }
 if(htim->Instance == TIM3){
 	CS43_SetVolume(potencjometr_255_int);//0-255
 	if(volume!=old_volume)
	{
		ssd1331_display_num(38, 12, old_volume, sizeof(old_volume), FONT_1608, BLACK);
		ssd1331_display_num(38, 12, volume, sizeof(volume), FONT_1608, GREEN);
		old_volume = volume;
	}
  }

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  tracks[0] = "Metin2.wav";
  tracks[1] = "Mario.wav";
  tracks[2] = "Polo.wav";
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_USB_HOST_Init();
  MX_FATFS_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	ssd1331_init();
	ssd1331_clear_screen(BLACK);
	ssd1331_display_string(0, 15, "GLOSNOSC:", FONT_1206, GREEN);
	ssd1331_display_string(0, 30, "CZAS:", FONT_1206, BLUE);
	ssd1331_display_string(30, 45, "2020", FONT_1206, YELLOW);

	CS43_Init(hi2c1, MODE_I2S);
	  CS43_SetVolume(180);//0-255
	  CS43_Enable_RightLeft(CS43_RIGHT_LEFT);

	  audioI2S_setHandle(&hi2s3);

	  bool isSdCardMounted=0;
	  bool pauseResumeToggle=0;

	  HAL_ADC_Start_IT(&hadc2);
	  HAL_TIM_Base_Start_IT(&htim3);
	  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    if(Appli_state == APPLICATION_START)
        {
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        }
        else if(Appli_state == APPLICATION_DISCONNECT)
        {
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
          f_mount(NULL, (TCHAR const*)"", 0);
          isSdCardMounted = 0;

        }

        if(Appli_state == APPLICATION_READY)
        {
            seconds = 0;
            tens_of_seconds = 0;
            minutes = 0;

          if(!isSdCardMounted)
          {
            f_mount(&USBHFatFS, (const TCHAR*)USBHPath, 0);
            isSdCardMounted = 1;
          }
          if(!HAL_GPIO_ReadPin(start_button_GPIO_Port, start_button_Pin))
          {

              if(actually_track>2)
              {
              	actually_track = 0;
              }
              else if(actually_track<0)
              {
            	  actually_track = 2;
              }
              if(last_track>2)
              {
            	  last_track = 0;
              }
              else if(last_track<0)
              {
            	  last_track = 2;
              }
           ssd1331_display_string(0, 0, tracks[last_track], FONT_1206, BLACK);
           ssd1331_display_string(0, 0, tracks[actually_track], FONT_1206, RED);

           HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

            HAL_Delay(500);
            wavPlayer_fileSelect(tracks[actually_track]);

            wavPlayer_play();
            is_playing = true;


            while(!wavPlayer_isFinished())
            {
              wavPlayer_process();

              if(!HAL_GPIO_ReadPin(next_right_GPIO_Port, next_right_Pin))
              {
            	  actually_track++;
            	  last_track = actually_track;
            	  last_track--;
            	  wavPlayer_stop();
              }

              if(!HAL_GPIO_ReadPin(next_left_GPIO_Port, next_left_Pin))
              {
            	  actually_track--;
            	  last_track = actually_track;
            	  last_track++;
            	  wavPlayer_stop();
              }

              if(!HAL_GPIO_ReadPin(start_button_GPIO_Port, start_button_Pin))
              {
                pauseResumeToggle^=1;
                if(pauseResumeToggle)
                {

                  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                  wavPlayer_pause();
                  is_playing = false;
                  HAL_Delay(500);
                }
                else
                {
                  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
                  HAL_Delay(1000);
                  if(!HAL_GPIO_ReadPin(start_button_GPIO_Port, start_button_Pin))
                  {

                    wavPlayer_stop();
                  }

                	  is_playing = true;
                    wavPlayer_resume();

                }
              }
            }

            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
            HAL_Delay(1000);
          }
        }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 271;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
