/**44444 
  *@file Driver_can.c
  *@date 2016-12-12
  *@author Albert.D
  *@brief 
  */

#include "Driver_can.h"
#include "Motor_Chassis.h"
//#include "Motor_Shoot.h"

uint8_t can1_rx_data[8];
uint8_t can2_rx_data[8];

uint8_t MotorTxData[8] = {0};    //���̿���
//uint8_t HeadTxData[8] = {0};     //��̨����
uint8_t TestTxData[8] = {0};     //���̵��ң�ؿ���ʵ��


uint16_t motor_angle_0x201, motor_angle_0x202, motor_angle_0x203, motor_angle_0x204;
int16_t RealSpeedLF, RealSpeedRF, RealSpeedLB, RealSpeedRB;  //���̵���ķ����ٶ�
//int16_t RealSpeedPLUCK=0, RealSpeedPLUCK2, RealSpeedPLUCK3;
//uint16_t RealAngleYAW, RealAnglePITCH, RealAnglePLUCK;


//can filter must be initialized before use
void CanFilter_Init(CAN_HandleTypeDef* hcan)   //Can��ʼ��
{
  CAN_FilterConfTypeDef canfilter;   //can�˲�
  
  //create memory to save the message, if not will raise error
  static CanTxMsgTypeDef  Tx1Message;
  static CanRxMsgTypeDef  Rx1Message;
  static CanTxMsgTypeDef  Tx2Message;
  static CanRxMsgTypeDef  Rx2Message;
  
  canfilter.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilter.FilterScale = CAN_FILTERSCALE_32BIT;
  
  //filtrate any ID you want here
//1��4���ù�������32λid��32λmask id ���ֱ�ͨ��2��16λ�����
  canfilter.FilterIdHigh = 0x0000;
  canfilter.FilterIdLow = 0x0000;
  canfilter.FilterMaskIdHigh = 0x0000;
  canfilter.FilterMaskIdLow = 0x0000;
  
  canfilter.FilterFIFOAssignment = CAN_FilterFIFO0;   //����FIFO���˲����Ĺ��˹�ϵ
  canfilter.FilterActivation = ENABLE;  //������˲���
  canfilter.BankNumber = 14;  //���˫CAN��STM32��Ʒ������CAN2��ʹ�õĹ���������ʼ��ţ�Ĭ��ֵ��14
  
  //use different filter for can1&can2
  if(hcan == &hcan1)
  {
    canfilter.FilterNumber = 0;  //ѡ��ĳ����������г�ʼ�������ý��չ����� ˫CAN ֵΪ0-27
    hcan->pTxMsg = &Tx1Message;
    hcan->pRxMsg = &Rx1Message;
  }
  if(hcan == &hcan2)
  {
    canfilter.FilterNumber = 14;
    hcan->pTxMsg = &Tx2Message;
    hcan->pRxMsg = &Rx2Message;
  }
  
  HAL_CAN_ConfigFilter(hcan, &canfilter);
  
}

extern uint8_t contro_flag,Head_flag;
//extern uint16_t aim_angle_206,aim_angle_205;

//��ȡ/ˢ��can����������
//it will be auto callback when can receive msg completely
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{

	  can1_rx_data[0] = hcan->pRxMsg->Data[0];
	  can1_rx_data[1] = hcan->pRxMsg->Data[1];
	  can1_rx_data[2] = hcan->pRxMsg->Data[2];
	  can1_rx_data[3] = hcan->pRxMsg->Data[3];

	
	 switch(hcan->pRxMsg->StdId)
     {
		 
		 case 0x201:
			{
			  	motor_angle_0x201= (can1_rx_data[0]<<8)|(can1_rx_data[1]);
			    RealSpeedLF= (can1_rx_data[2]<<8)|(can1_rx_data[3]);
				
			}break;
			
			case 0x202:
			{
				motor_angle_0x202= (can1_rx_data[0]<<8)|(can1_rx_data[1]);
				RealSpeedRF= (can1_rx_data[2]<<8)|(can1_rx_data[3]);
				
			}break;
			
			case 0x203:
			{
				motor_angle_0x203= (can1_rx_data[0]<<8)|(can1_rx_data[1]);
				RealSpeedRB= (can1_rx_data[2]<<8)|(can1_rx_data[3]);			
			}break;
			case 0x204:            //����LB
			{
				motor_angle_0x204= (can1_rx_data[0]<<8)|(can1_rx_data[1]);
				RealSpeedLB= (can1_rx_data[2]<<8)|(can1_rx_data[3]);				
			}break;   
  }
     
	   //���ݽ��յ���ID�ŷ�������֡ 
    switch(hcan->pRxMsg->StdId)
   {
     case 0x201:
     case 0x202:
     case 0x203:
     case 0x204:
      if(contro_flag==1)
      {
          Control_ChassisPID();
          contro_flag = 0;   
      }break;   
   }
      HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
}

//CAN send message test
void CAN_Send_Msg(CAN_HandleTypeDef* hcan, uint8_t *msg, uint32_t id, uint8_t len)
{
  uint8_t index = 0;
  
  hcan->pTxMsg->StdId = id;
  hcan->pTxMsg->IDE = CAN_ID_STD;
  hcan->pTxMsg->RTR = CAN_RTR_DATA;
  hcan->pTxMsg->DLC = len;
  
  for(index = 0; index <len; index++)
    hcan->pTxMsg->Data[index] = msg[index];
  
  HAL_CAN_Transmit_IT(hcan);

}
