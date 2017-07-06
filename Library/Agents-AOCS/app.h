/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Gate.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/utils/Load.h>
#include <xdc/runtime/Log.h>
#include <ti/uia/events/UIABenchmark.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/gates/GateMutexPri.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "maes.h"
#include "arm_math.h"
#include <math.h>

using namespace MAES;
/*To use the sensor in c++*/
extern "C" {
#include <app_files/driver/bmi160_support.h>
}
#ifndef APP_FILES_APP_H_
#define APP_FILES_APP_H_

#define GET_MEASUREMENTS    1
#define PRIORI_EST          2
#define KALMAN_GAIN         3
#define POST_EST            4
#define UPDATE              5
/***********************************************************************
*                Sensors-related functions/variables                   *
***********************************************************************/
/* Mutex variable */
GateMutexPri_Handle i2cGate;
GateMutexPri_Params gateParams;
IArg gatekey;

/*Sensor Board Variables*/
I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;

UART_Handle     uart;
UART_Params     uartParams;
UART_Callback   uartCallback;

/*RX and TX Buffers for I2C Protocol*/
uint8_t         txBuffer[10];
uint8_t         rxBuffer[30];
/*String buffer for snprintf function*/
char            stringBuffer[295];

//Calibration off-sets
int8_t accel_off_x, accel_off_y, accel_off_z;
int16_t gyro_off_x, gyro_off_y, gyro_off_z;

// BMI160/BMM150
struct bmi160_gyro_t        s_gyroXYZ;
struct bmi160_accel_t       s_accelXYZ;
struct bmi160_mag_xyz_s32_t s_magcompXYZ;

void sensor(UArg arg0, UArg arg1);
void uartFxn(UArg arg0, UArg arg1);

Agent_AID UART_AID,Sensor_AID,Kalman_AID;

/***********************************************************************
*                Kalman-related functions/variables                   *
***********************************************************************/
/*
 * x: State matrix: [q bacc bmag]^T. q:[q1 q2 q3 q4]^T q4: scalar
 * Omega: Discretized transition matrix
 * P: Error covariance matrix
 * Q: Process noise covariance matrix
 * R: Covariance matrix of measurement model
 * z: Measurement vector
 * F: Jacobian matrix of the lineariazed z
 * f: Predicted value with the estimated x value (zk+1-f(xk+1))
 * g: Predicted gravity force. Constant
 * m: Predicted magnetic field. Constant
 * K: Kalman gain
 * sigmaWa: Random walk rate for accelerometer
 * sigmaWm: Random walk rate for magnetometer
 * sigma_a: White Noise for accelerometer
 * sigma_m: White Noise for magnetometer
 * sigma_g: White Noise for gyro
 * Eg: Covariance matrix sigma_g^2*Identity
*/

/*Defining arrays data storage*/
float32_t xData[10];
float32_t OmegaData[16];
float32_t PhiData[100];
float32_t PData[100];
float32_t QData[100];
float32_t RData[36];
float32_t KData[60];
float32_t zData[6];
float32_t fData[6];
float32_t FData[60];
float32_t gData[3];
float32_t hData[3];
float32_t XiData[12];
float32_t EgData[9];
float32_t sigmagData[3];
float32_t sigmaaData[3];
float32_t sigmamData[3];
float32_t sigmaWaData[3];
float32_t sigmaWmData[3];
float32_t gyro_measData[3];
float32_t gyro_meas_nextData[3];

/*Defining matrix instances for operations*/
arm_matrix_instance_f32 x,Omega,Phi,P,Q,R,K,z,f,F,g,h,Xi,Eg;
arm_matrix_instance_f32 sigmaWa,sigmaWm,sigmag,sigmaa,sigmam;
arm_matrix_instance_f32 gyro_meas, gyro_meas_next;


float32_t roll, yaw, pitch;
float32_t magSens,gyroSens,accSens,ts;

class supporting_functions{
public:
    void init();
    void updateQ();
    void get_measurements();
    void updatePhi();
    arm_matrix_instance_f32 getRotation();
    void updatef();
    void updateJacobian();
    void normalizing();
    void updateR();

};

void kalman(UArg arg0, UArg arg1);
void print(float32_t t);
#endif /* APP_FILES_APP_H_ */
