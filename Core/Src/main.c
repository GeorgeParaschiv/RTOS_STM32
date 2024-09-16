///* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file           : main.c
//  * @brief          : Main program body
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2023 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
///* USER CODE END Header */
///* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!
//#include "common.h"
//#include "k_mem.h"
//#include "k_task.h"
//
///**
//  * @brief  The application entry point.
//  * @retval int
//  */
//
//void Task1(void *) {
//	while(1){
//		printf("1\r\n");
//		for (uint32_t i_cnt = 0; i_cnt < 5000; i_cnt++);
//		osYield();
//	}
//}
//
//void Task2(void *) {
//	while(1){
//		printf("2\r\n");
//		for (uint32_t i_cnt = 0; i_cnt < 5000; i_cnt++);
//		osYield();
//	}
//}
//
//
//void Task3(void *) {
//	while(1){
//		printf("3\r\n");
//		for (uint32_t i_cnt = 0; i_cnt < 5000; i_cnt++);
//		osYield();
//	}
//}
//
//
//int main(void)
//{
//
//  /* MCU Configuration: Don't change this or the whole chip won't work!*/
//
//  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();
//  /* Configure the system clock */
//  SystemClock_Config();
//
//  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_USART2_UART_Init();
//  /* MCU Configuration is now complete. Start writing your code below this line */
//
//  printf("START\r\n");
//
//  osKernelInit();
//
//  TCB st_mytask;
//  st_mytask.stack_size = STACK_SIZE;
//
//  st_mytask.ptask = &Task1;
//  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &Task2;
//  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &Task3;
//  osCreateTask(&st_mytask);
//
//  osKernelStart();
//
//  /* Infinite loop */
//  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//  }
//  /* USER CODE END 3 */
//}
//
#include "common.h"
#include "k_task.h"
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!


int i_test = 0;

void Task1(void *) {
	i_test++;
	osSleep(1);

	//instead of a while loop, keep recreating itself and exiting
	TCB st_mytask;
	st_mytask.ptask = &Task1;
	st_mytask.stack_size = 0x400;
	osCreateTask(&st_mytask);
	osTaskExit();
}

void Task2(void *) {
	while(1){
		printf("Back to you %d\r\n",i_test);
		osYield();
	}
}

int main(void) {
  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */

  osKernelInit();

  TCB st_mytask;
  st_mytask.stack_size = 0x400;

  st_mytask.ptask = &Task1;
  osCreateTask(&st_mytask);

  st_mytask.ptask = &Task2;
  osCreateTask(&st_mytask);

  osKernelStart();

  while (1);
}
