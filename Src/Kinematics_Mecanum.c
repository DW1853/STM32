/**************************************************************************
文件版本：神炽焰 V1.0
文件功能：
1、麦克纳姆轮运动学逆解函数
2、麦克纳姆轮运动学正解函数
**************************************************************************/
#include "Kinematics_Mecanum.h"
#include "ShareWare.h"

/**************************************************************************
函数功能：麦克纳姆轮运动学逆解函数
入口参数：车体运动学结构体参数
返回  值：通过输入车体的线速度与角速度计算出4个电机的转速
**************************************************************************/
void Kinematics_Mecanum_CalculateRPM(Kinematics_Struct *Kinematics)
{
	float linear_vel_x_mins;
	float linear_vel_y_mins;
	float angular_vel_z_mins;
	float tangential_vel;
	float x_rpm;
	float y_rpm;
	float tan_rpm;

	//convert m/s to m/min
	linear_vel_x_mins = Kinematics->Linear_X * 60;
	linear_vel_y_mins = Kinematics->Linear_Y * 60;
	
	//convert rad/s to rad/min
	angular_vel_z_mins = Kinematics->Angular_Z * 60;

	tangential_vel = angular_vel_z_mins * (((float)FR_WHEELS_DISTANCE / 2) + ((float)LR_WHEELS_DISTANCE / 2));

	x_rpm = (float)linear_vel_x_mins / WHEEL_CIRCUMFERENCE;
	y_rpm = (float)linear_vel_y_mins / WHEEL_CIRCUMFERENCE;
	tan_rpm = (float)tangential_vel / WHEEL_CIRCUMFERENCE;

	//calculate for the target motor RPM and direction
	Kinematics->M1_RPM = x_rpm - y_rpm - tan_rpm;//M1电机目标转速

	Kinematics->M2_RPM = x_rpm + y_rpm + tan_rpm;//M2电机目标转速
	
	Kinematics->M3_RPM = x_rpm + y_rpm - tan_rpm;//M3电机目标转速

	Kinematics->M4_RPM = x_rpm - y_rpm + tan_rpm;//M4电机目标转速
}

/**************************************************************************
函数功能：麦克纳姆轮运动学正解函数
入口参数：车体运动学结构体参数
返回  值：通过输入4个电机的转速计算出车体的线速度与角速度
**************************************************************************/
void Kinematics_Mecanum_GetVelocities(Kinematics_Struct *Kinematics)
{
	float average_rps_x;
	float average_rps_y;
	float average_rps_a;

	//convert average revolutions per minute to revolutions per second
	average_rps_x = (float)((float)(Kinematics->M1_RPM + Kinematics->M2_RPM + Kinematics->M3_RPM + Kinematics->M4_RPM) / 4) / 60; // RPM
	Kinematics->Linear_X = average_rps_x * WHEEL_CIRCUMFERENCE; // m/s
	
	//convert average revolutions per minute in y axis to revolutions per second
	average_rps_y = (float)((float)(-Kinematics->M1_RPM + Kinematics->M2_RPM + Kinematics->M3_RPM - Kinematics->M4_RPM) / 4) / 60; // RPM
	Kinematics->Linear_Y = average_rps_y * WHEEL_CIRCUMFERENCE; // m/s
	
	average_rps_a = (float)((float)(-Kinematics->M1_RPM + Kinematics->M2_RPM - Kinematics->M3_RPM + Kinematics->M4_RPM) / 4) / 60;
	Kinematics->Angular_Z =  (float)(average_rps_a * WHEEL_CIRCUMFERENCE) / (((float)FR_WHEELS_DISTANCE / 2) + ((float)LR_WHEELS_DISTANCE / 2)); //  rad/s
}
