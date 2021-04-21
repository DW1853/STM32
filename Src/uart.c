#include "uart.h"
#include <stdarg.h>
#include "ShareWare.h"

UART_HandleTypeDef huart2;
__IO uint8_t Usart2_TX_Buf[Usart2_TX_LEN];//串口2发送数据数组
__IO uint8_t Usart2_RX_Buf[Usart2_RX_LEN];//串口2接收数据数组
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

UART_HandleTypeDef huart4;
__IO uint8_t Uart4_TX_Buf[Uart4_TX_LEN];
__IO uint8_t Uart4_RX_Buf[Uart4_RX_LEN];
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;

/*-----------------------------------------------------------
函数功能: 串口2初始化函数
输入参数: None
返 回 值: None
说    明: 配置串口2参数 波特率115200
 -----------------------------------------------------------*/
void MX_USART2_Init(void)
{
  huart2.Instance = USART2;//串口2
  huart2.Init.BaudRate = 230400;//波特率
  huart2.Init.WordLength = UART_WORDLENGTH_8B;//8位数据位
  huart2.Init.StopBits = UART_STOPBITS_1;//1位停止位
  huart2.Init.Parity = UART_PARITY_NONE;//无校验
  huart2.Init.Mode = UART_MODE_TX_RX;//全双工串口模式
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
		
	HAL_UART_Receive_DMA(&huart2, (uint8_t*)Usart2_RX_Buf, Usart2_RX_LEN);//串口DMA接收
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//开启空闲接收中断
}

/*-----------------------------------------------------------
函数功能: 串口4初始化函数
输入参数: None
返 回 值: None
说    明: 配置串口4参数 波特率100000
 -----------------------------------------------------------*/
void MX_UART4_Init(void)
{
  huart4.Instance = UART4;//串口4
  huart4.Init.BaudRate = 100000;//波特率
  huart4.Init.WordLength = UART_WORDLENGTH_9B;//8位数据位，1位校验位
  huart4.Init.StopBits = UART_STOPBITS_1;//1位停止位
  huart4.Init.Parity = UART_PARITY_EVEN;//偶校验
  huart4.Init.Mode = UART_MODE_TX_RX;//全双工串口模式
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart4);

	HAL_UART_Receive_DMA(&huart4, (uint8_t*)Uart4_RX_Buf, Uart4_RX_LEN);//串口DMA接收
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);//开启空闲接收中断
}

/*-----------------------------------------------------------
函数功能: 串口接收空闲中断函数
输入参数: 串口句柄类型指针
返 回 值: None
说    明: 串口接收到数据后一个byte的高电平(空闲)状态就会触发空闲中断进入该函数，
					用户在函数中读取串口接收到的数据
 -----------------------------------------------------------*/
void UsartReceive_IDLE(UART_HandleTypeDef *huart)//串口接收空闲中断函数
{
	uint32_t temp = 0;
	if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))  
	{			
		/*------------Usart2-------------*/
		if(huart->Instance == USART2)
		{
			__HAL_UART_CLEAR_IDLEFLAG(huart);
			temp = huart->Instance->SR;
			temp = huart->Instance->DR;
			temp = hdma_usart2_rx.Instance->CNDTR; 
			HAL_UART_DMAStop(huart);
		
			RingBuff_Write(&RingBuff_Mast, (uint8_t*)Usart2_RX_Buf, Usart2_RX_LEN-temp);//往环形缓冲区写入一组数据函数
			
			HAL_UART_Receive_DMA(huart, (uint8_t*)Usart2_RX_Buf, Usart2_RX_LEN);//串口DMA接收
		}
		/*-------------Uart4--------------*/
		else if(huart->Instance == UART4)
		{
			__HAL_UART_CLEAR_IDLEFLAG(huart);
			temp = huart->Instance->SR;
			temp = huart->Instance->DR;
			temp = hdma_uart4_rx.Instance->CNDTR; 
			HAL_UART_DMAStop(huart);
			
			RC_Update(&RC, (uint8_t*)Uart4_RX_Buf);//遥控器数据解析函数
			
			HAL_UART_Receive_DMA(huart, (uint8_t*)Uart4_RX_Buf, Uart4_RX_LEN);//串口DMA接收
		}
	}
}

/*-----------------------------------------------------------
函数功能: 串口接收完成中断函数
输入参数: 串口句柄类型指针
返 回 值: None
说    明: 串口接收完数据后中断进入该函数，用户在函数中读取串口接收到的数据
 -----------------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)//串口接收中断
{	

}

/*-----------------------------------------------------------
函数功能: 串口发送完成中断函数
输入参数: 串口句柄类型指针
返 回 值: None
说    明: 串口发送完数据后中断进入该函数
 -----------------------------------------------------------*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)//串口发送中断
{

}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	/*如文档说明，要清除ORE中断需要按顺序读取USART_SR和USART_DR寄存器！*/
  uint32_t isrflags   = READ_REG(huart->Instance->SR);//手册上有讲，清错误都要先读SR
	if((__HAL_UART_GET_FLAG(huart, UART_FLAG_PE))!=RESET)
	{
		READ_REG(huart->Instance->DR);//第二步读DR
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);//PE清标志	
	}
	if((__HAL_UART_GET_FLAG(huart, UART_FLAG_FE))!=RESET)
	{
		READ_REG(huart->Instance->DR);//第二步读DR
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);//FE清标志
	}
        
	if((__HAL_UART_GET_FLAG(huart, UART_FLAG_NE))!=RESET)
	{
		READ_REG(huart->Instance->DR);//第二步读DR
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);//NE清标志
  }        
        
  if((__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE))!=RESET)
  {
    READ_REG(huart->Instance->CR1);//第二步读CR
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);//ORE清标志
  }      
}


void dma_printf(const char *format, ...)//DMA发送方式printf()函数
{
	uint32_t length;
	va_list args;
	va_start(args, format);
	length = vsnprintf((char *)Usart2_TX_Buf, sizeof(Usart2_TX_Buf), (char *)format, args);
	va_end(args);
	
	HAL_UART_Transmit_DMA(&huart2, (uint8_t *)Usart2_TX_Buf, length);
}
