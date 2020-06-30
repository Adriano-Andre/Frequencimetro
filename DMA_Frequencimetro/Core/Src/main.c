/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           main.c
  * @author 		Adriano André
  * @date			mai/2020
  * @version		1.0v
  * @brief          Medidor de período e frequência.
  *	Utilizando o modo Input Capture, construa uma aplicação capaz de medir o período de
  *	um pulso, com uma resolução de tempo de 1 μs.
  *	Exiba o resultado através de um terminal serial, por exemplo.
  *	Utilize DMA para fazer a leitura dos tempos das 2 bordas.
  ******************************************************************************
  */
/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSG_SIZE       100			///<tamanho da string message
#define EDGES           2			///<número de bordas a serem lidas
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t period;					///<periodo medido do sinal injetado
uint32_t frequence;					///<frequencia medida do sinal injetado
volatile uint32_t measure[EDGES];	///<medida das bordas do sinal
enum TEDGE 							///<enumeração para controle do vetor de bordas
	{
	previous,
	current
	};
char message[MSG_SIZE];      	///<string usada para envio dos dados pela serial
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  //!ativação dos timers
  HAL_TIM_IC_Start_DMA(&htim5, TIM_CHANNEL_2, (uint32_t*) measure, EDGES);	//ligando o timer 5 (CH2) em modo input capture com DMA
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);									//ligando o timer 2 (CH1) em modo output compare (gera sinal)
  HAL_TIM_Base_Start(&htim10);												//ligando o timer 10 para envio de dados pela usart2
  __HAL_TIM_CLEAR_FLAG(&htim10,TIM_FLAG_UPDATE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //!envio dos dados coletados
	  if(__HAL_TIM_GET_FLAG(&htim10,TIM_FLAG_UPDATE))
	  {
	  __HAL_TIM_CLEAR_FLAG(&htim10,TIM_FLAG_UPDATE);
	  sprintf(message,
			  "Borda anterior=%li\tBorda atual=%li\tPeriodo=%li us\tFrequencia=%li Hz\n\r",
			  measure[previous],measure[current],period,frequence);
	  HAL_UART_Transmit_DMA(&huart2, (uint8_t*)&message, strlen(message));
	  }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
 * @brief Calcula o periodo do sinal lido
 * @param htim handler do timer que requisitou a interrupção
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	period=measure[current]-measure[previous];						//calculando período
	frequence=(1/(period*0.000001));								//calculando frequência
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
