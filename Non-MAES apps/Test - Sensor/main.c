/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Gate.h>
#include <xdc/runtime/Error.h>


/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
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
#include "math.h"
#include "bmi160_support.h"


/* General Defines */
//RX and TX Buffers for I2C Protocol
uint8_t         txBuffer[10] = {0};
uint8_t         rxBuffer[30] = {0};
//String buffer for snprintf function
char			stringBuffer[295];

//Calibration off-sets
int8_t accel_off_x;
int8_t accel_off_y;
int8_t accel_off_z;
int16_t gyro_off_x;
int16_t gyro_off_y;
int16_t gyro_off_z;

// Task stack size setup

#define BMI_TASK_STACK_SIZE  				2048
#define INIT_TASK_STACK_SIZE 				2048
#define UART_SEND_TASK_STACK_SIZE 			2048
#define MAIN_THREAD_TASK_STACK_SIZE			2048

static Char BMITaskStack[BMI_TASK_STACK_SIZE];
static Char UartSendTaskStack[UART_SEND_TASK_STACK_SIZE];
static Char MainThreadTaskStack[MAIN_THREAD_TASK_STACK_SIZE];



/* Global Variables */
Task_Handle initSensors, readBMI;
Task_Handle uartSend, mainThread;
Task_Params taskParams;

Semaphore_Handle BMISem, uartSem;

GateMutexPri_Handle i2cGate;
GateMutexPri_Params gateParams;
IArg gatekey;

Error_Block eb;

I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;

UART_Handle 	uart;
UART_Params 	uartParams;
UART_Callback	uartCallback;


// BMI160/BMM150
struct bmi160_gyro_t        s_gyroXYZ;
struct bmi160_accel_t       s_accelXYZ;
struct bmi160_mag_xyz_s32_t s_magcompXYZ;

//Sensor Status Variables
bool BMI_on = true;

/***********************************************************
  Funtion:

   Main thread handler for system. Determines when to update
   sensor values using a counter, what gesture should be displayed
   in the GUI, and when to send over UART.

 ***********************************************************/
void mainThreadHandler(UArg arg0, UArg arg1)
{
    Uint32 timeBMI=0;
    I2C_Params_init(&i2cParams);
      i2cParams.bitRate = I2C_400kHz;
      i2cParams.transferMode = I2C_MODE_BLOCKING;
      i2cParams.transferCallbackFxn = NULL;
      i2c = I2C_open(Board_I2C0, &i2cParams);
      if (i2c == NULL) {
          System_abort("Error Initializing I2C\n");
      }
      else {
          System_printf("I2C Initialized!\n");
      }
      System_flush();

      /* Initialize UART */
      UART_Params_init(&uartParams);
      uartParams.writeDataMode = UART_DATA_BINARY;
      uartParams.baudRate = 115200;
      uart = UART_open(Board_UART0, &uartParams);
      if(uart == NULL) {
          System_abort("UART open failed...\n");
      }
      else {
          System_printf("UART Initialized!\n");
      }
      System_flush();

      /* Initialize the BMI Sensor */
      bmi160_initialize_sensor();
      //bmi160_config_running_mode(APPLICATION_NAVIGATION);
      bmi160_config_running_mode(STANDARD_UI_9DOF_FIFO);

      bmi160_accel_foc_trigger_xyz(0x03, 0x03, 0x01, &accel_off_x, &accel_off_y, &accel_off_z);
      bmi160_set_foc_gyro_enable(0x01, &gyro_off_x, &gyro_off_y, &gyro_off_z);

      System_printf("Sensors initialized!\n");
      System_flush();

      while(1) {
	    //Accel and Gyro update frequency
		if ((abs(Clock_getTicks()-timeBMI)>=100) &&BMI_on) //100:100000us, 1000 is 1 seconds 1000us*1000 (since system tick is 1000us)
		{
		    timeBMI=Clock_getTicks();
			//Read Accel and Gyro values
			Semaphore_post(BMISem);
			Semaphore_post(uartSem);
		}

  	}
}

/***********************************************************
  Funtion:

   Task handling the reading of the BMI160 sensor.
   Multiplying -1 to Y and Z axis of BMI160 sensor to match
   BMI150's axes

 ***********************************************************/
void taskBMI(UArg arg0, UArg arg1)
{
	while(1) {
		Semaphore_pend(BMISem, BIOS_WAIT_FOREVER);
		//Read Accel and Gyro values
		gatekey = GateMutexPri_enter(i2cGate);
		bmi160_read_accel_xyz(&s_accelXYZ);
		bmi160_read_gyro_xyz(&s_gyroXYZ);
		bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
		GateMutexPri_leave(i2cGate, gatekey);

		System_printf("Accel: %d (X), %d (Y), %d (Z)\n"
				"Gyro: %d (X), %d (Y), %d (Z)\n" "Mag: %d (X), %d (Y), %d (Z)\n",
				s_accelXYZ.x, -1*s_accelXYZ.y, -1*s_accelXYZ.z,
				s_gyroXYZ.x, -1*s_gyroXYZ.y, -1*s_gyroXYZ.z,
				s_magcompXYZ.x, s_magcompXYZ.y, s_magcompXYZ.z);
		System_flush();
	}
}


/***********************************************************
  Funtion:

   Task handling the formatting of JSON string to be sent over
   UART to the GUI.
   Multiplying -1 to Y and Z axis of BMI160 sensor to match
   BMI150's axes

 ***********************************************************/
Void uartFxn(UArg arg0, UArg arg1)
{
	while (1) {
		Semaphore_pend(uartSem, BIOS_WAIT_FOREVER);
		snprintf(stringBuffer, 295,
				"{\"gyro\":{\"x\":%d,\"y\":%d,\"z\":%d},"
				"\"accel\":{\"x\":%d,\"y\":%d,\"z\":%d},"
				"\"mag\":{\"x\":%d,\"y\":%d,\"z\":%d}}\n",
				s_gyroXYZ.x, -1*s_gyroXYZ.y, -1*s_gyroXYZ.z,
				s_accelXYZ.x, -1*s_accelXYZ.y, -1*s_accelXYZ.z,
				s_magcompXYZ.x, s_magcompXYZ.y, s_magcompXYZ.z);
		gatekey = GateMutexPri_enter(i2cGate);
		UART_write(uart, stringBuffer, strlen(stringBuffer));
		GateMutexPri_leave(i2cGate, gatekey);
	}
}

/***********************************************************
  Funtion:

   Function called before BIOS_Start() to dynamically initialize
   and allocate the proper stack sizes for each task.

 ***********************************************************/
void AP_createTask(void)
{
	// Configure tasks

	Task_Params_init(&taskParams);
	taskParams.stack = BMITaskStack;
	taskParams.stackSize = BMI_TASK_STACK_SIZE;
	taskParams.priority = 2;
	readBMI = Task_create(taskBMI, &taskParams, NULL);

	Task_Params_init(&taskParams);
	taskParams.stack = UartSendTaskStack;
	taskParams.stackSize = UART_SEND_TASK_STACK_SIZE;
	taskParams.priority = 2;
	uartSend = Task_create(uartFxn, &taskParams, NULL);

	Task_Params_init(&taskParams);
	taskParams.stack = MainThreadTaskStack;
	taskParams.stackSize = MAIN_THREAD_TASK_STACK_SIZE;
	taskParams.priority = 1;
	mainThread = Task_create(mainThreadHandler, &taskParams, NULL);
}

/***********************************************************
  Funtion:

   Function called before BIOS_Start() to dynamically initialize
   each semaphore.

 ***********************************************************/
void AP_createSemaphore(void) {

    BMISem = Semaphore_create(0, NULL, &eb);
	if(BMISem == NULL) {
		System_abort("BMI semaphore create failed");
	}

	uartSem = Semaphore_create(0, NULL, &eb);
	if(uartSem == NULL) {
		System_abort("UART semaphore create failed");
	}

}

/***********************************************************
  Funtion:

   Function called before BIOS_start() to create the Gate
   Mutex used for the isolation of the I2C and UART peripherals.

 ***********************************************************/
void AP_createGateMutex(void) {
	GateMutexPri_Params_init(&gateParams);

	i2cGate = GateMutexPri_create(&gateParams, &eb);
	if(i2cGate == NULL) {
		System_abort("Gate create failed");
	}
}

/* ======== main ========*/
int main(void){
	/* Call board init functions */
	Board_initGeneral();
	Board_initGPIO();
	Board_initI2C();
	Board_initUART();

	/* Turn on user LED */
	GPIO_write(Board_LED0, Board_LED_ON);

	Error_init(&eb);

	AP_createTask();
	AP_createSemaphore();
	AP_createGateMutex();

	System_flush();
	/* Start BIOS */
	BIOS_start();

	return (0);
}
