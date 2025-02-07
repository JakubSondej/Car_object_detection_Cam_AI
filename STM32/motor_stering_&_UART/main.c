/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"      //to bibliteka np do typoów danych

#include "bib.h"  //to moja bibliteka

#include "INA3221.h"

#include "stdio.h"  //to potrzebne do uart
#include "string.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t znak='z';
uint8_t znak_old;

char rx_buffer[100];
char parsed_velocity;
char parsed_direction;


WHEEL_Type Front_Right;  //tworzę 4 obiekty - 4 koła, jako obiekty globalne
WHEEL_Type Front_Left;
WHEEL_Type Back_Right;
WHEEL_Type Back_Left;

INA3221_Type Sensor_INA3221; //tworze obiekt mojego czujnika do pomiaru pradu i napiecia

int speed=0; // to zmienna która przechowuje żądaną predkość
float sp1=0;
float sp2=0;
float sp3=0;
float sp4=0;

int x1=0;
int x2=0;
int x3=0;
int x4=0;

INA3221_HandleTypeDef ina3221;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM11_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_TIM10_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  InitFunction(&Front_Left, &Front_Right, &Back_Left, &Back_Right);   //to wywołanie funkcji inicjalizujacej 4 koła-są w niej też starty timerów
  HAL_TIM_Base_Start_IT(&htim10);//start podstawy czasu w trybie przerwaniowym - co 0.5s
  HAL_TIM_Base_Start_IT(&htim11);//start podstawy czasu w trybie przerwaniowym - co 0.2s

  INA3221_Init(&ina3221, &hi2c1, 0x40);
  INA3221_SetShuntResistor(&ina3221, INA3221_CHANNEL_1, 0.01f); // 10 mΩ
  INA3221_SetShuntResistor(&ina3221, INA3221_CHANNEL_2, 0.01f); // 10 mΩ
  INA3221_SetShuntResistor(&ina3221, INA3221_CHANNEL_3, 0.01f); // 10 mΩ

  Sensor_INA3221.TotalEnergy=0;
  //HAL_UART_Receive_IT(&huart2, (uint8_t *)rx_buffer, sizeof(rx_buffer));
  HAL_UART_Receive_IT(&huart2, &znak,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  sp1=Front_Left.encoder_data.CurrentVelocity;
	  sp2=Front_Right.encoder_data.CurrentVelocity;
	  sp3=Back_Left.encoder_data.CurrentVelocity;
	  sp4=Back_Right.encoder_data.CurrentVelocity;
	  x1=Front_Left.pid_data.PidSignal;
	  x2=Front_Right.pid_data.PidSignal;
	  x3=Back_Left.pid_data.PidSignal;
	  x4=Back_Right.pid_data.PidSignal;

	  //Logika(&Front_Left, &Front_Right, &Back_Left, &Back_Right);

	  //wywołuję funkcję która ustawia predkosć i kierunek jazdy dla czterech kół
	  MotorState(&Front_Left);
	  MotorState(&Front_Right);
	  MotorState(&Back_Left);
	  MotorState(&Back_Right);

	  VelocityCalculation(&Front_Left, &Front_Right, &Back_Left, &Back_Right,speed);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM11_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM11_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM11_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* EXTI0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* TIM10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM10_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM10_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==Front_Left.pinout_data.EncoderPin)               //sprawdzam z którego pinu przyszło przerwanie i inkrementuję odpowiednią zmiennę od odpowiedniego koła
	{
		PositionIncrement(&Front_Left);
	}
	if(GPIO_Pin==Front_Right.pinout_data.EncoderPin)
	{
		PositionIncrement(&Front_Right);
	}
	if(GPIO_Pin==Back_Left.pinout_data.EncoderPin)
	{
		PositionIncrement(&Back_Left);
	}
	if(GPIO_Pin==Back_Right.pinout_data.EncoderPin)
	{
		PositionIncrement(&Back_Right);
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//to callback od przerwania co określony czas
{
	if(htim->Instance==TIM11)//sprawdzam czy przerwanie pochodzi z timera który mnie interesuje - przerwanie co 0.2s
	{
		//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); //to część kodu testujacego
		EncoderVelocity(&Front_Left);
		EncoderVelocity(&Front_Right);
		EncoderVelocity(&Back_Left);
		EncoderVelocity(&Back_Right);

		SpeedRamp(&Front_Left);
		SpeedRamp(&Front_Right);
		SpeedRamp(&Back_Left);
		SpeedRamp(&Back_Right);

		PID(&Front_Left);
		PID(&Front_Right);
		PID(&Back_Left);
		PID(&Back_Right);

	}
	if(htim->Instance==TIM10)//sprawdzam czy przerwanie pochodzi z timera który mnie interesuje - przerwanie co 0.5s
	{
		//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); //to część kodu testujacego
		//odczytuję informację z czujnika i przypisuję do odpowiedniego miejsca w strukturze
		Sensor_INA3221.Channel1.Current=INA3221_GetCurrentRaw(&ina3221, INA3221_CHANNEL_1);
		Sensor_INA3221.Channel1.Voltage=INA3221_GetBusVoltage(&ina3221, INA3221_CHANNEL_1);
	    Sensor_INA3221.Channel2.Current=INA3221_GetCurrentRaw(&ina3221, INA3221_CHANNEL_2);
	    Sensor_INA3221.Channel2.Voltage=INA3221_GetBusVoltage(&ina3221, INA3221_CHANNEL_2);
		Sensor_INA3221.Channel3.Current=INA3221_GetCurrentRaw(&ina3221, INA3221_CHANNEL_3);
		Sensor_INA3221.Channel3.Voltage=INA3221_GetBusVoltage(&ina3221, INA3221_CHANNEL_3);

		Sensor_INA3221.Power=Sensor_INA3221.Channel3.Voltage*Sensor_INA3221.Channel3.Current;   //obliczam teraz moc w watach
		Sensor_INA3221.TotalEnergy+=Sensor_INA3221.Power/7200;                                   //teraz obliczam enegie zużytą w mWh
		//teraz wysyłanie informacj
		char Message[100];
		//to obroty kół na minutę
		float Velo=(abs(Front_Left.encoder_data.CurrentVelocity)+abs(Front_Right.encoder_data.CurrentVelocity)+abs(Back_Left.encoder_data.CurrentVelocity)+abs(Back_Right.encoder_data.CurrentVelocity))*15/54;
		int Lenght = sprintf((char*)Message,"V=%.2f,C1=%.2f,N1=%.2f,C2=%.2f,N2=%.2f,C3=%.2f,N3=%.2f,P=%.2f,E=%.2f\n\r",Velo
			,Sensor_INA3221.Channel1.Current,Sensor_INA3221.Channel1.Voltage,Sensor_INA3221.Channel2.Current,Sensor_INA3221.Channel2.Voltage,Sensor_INA3221.Channel3.Current,Sensor_INA3221.Channel3.Voltage,Sensor_INA3221.Power,Sensor_INA3221.TotalEnergy);
		HAL_UART_Transmit_IT(&huart2, (uint8_t*)Message, Lenght);  // MUSI być rzutowanie na (uint8_t*) dla Message bo jest char
		//zobacz sobie dla deafultowej funkcji 'HAL_UART_Transmit_IT' co jest podane
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
		{
		  HAL_UART_Receive_IT(&huart2, &znak,1);
		  //if(znak!=znak_old)

			  speed=ReadInfoFromUart(znak, &Front_Left, &Front_Right, &Back_Left, &Back_Right);


		}

}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
