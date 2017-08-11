/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>


/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "sensor_driver/bmi160_support.h"


/* General Defines */
//RX and TX Buffers for I2C Protocol
uint8_t         txBuffer[10] = {0};
uint8_t         rxBuffer[30] = {0};

//Calibration off-sets
int8_t accel_off_x;
int8_t accel_off_y;
int8_t accel_off_z;
int16_t gyro_off_x;
int16_t gyro_off_y;
int16_t gyro_off_z;

// Task stack size setup
#define TASKSTACKSIZE       2048

char MISSION_STACK[TASKSTACKSIZE];
char SENSOR_STACK[TASKSTACKSIZE];
char LED0_STACK[TASKSTACKSIZE];
char LED1_STACK[TASKSTACKSIZE];
Task_Handle uart_handle,led0_handle,led1_handle,sensor_handle;
Semaphore_Handle led0_sem, led1_sem, sensor_sem;
UART_Handle uart;
I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;

char response[100];
enum measurement{
    GYROSCOPE,
    ACCELEROMETER,
    MAGNETOMETER
}meas_type;

// BMI160/BMM150
struct bmi160_gyro_t        s_gyroXYZ;
struct bmi160_accel_t       s_accelXYZ;
struct bmi160_mag_xyz_s32_t s_magcompXYZ;

void MissionFxn(UArg arg0, UArg arg1)
{
    char input;
    char command[10];
    int counter;

    UART_Params uartParams;

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

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

    snprintf(response,100, "\fInitializing sensors. Wait.\r\n\0");
    UART_write(uart, response, strlen(response));
    /* Initialize the BMI Sensor */
    bmi160_initialize_sensor();
    //bmi160_config_running_mode(APPLICATION_NAVIGATION);
    bmi160_config_running_mode(STANDARD_UI_9DOF_FIFO);

    bmi160_accel_foc_trigger_xyz(0x03, 0x03, 0x01, &accel_off_x, &accel_off_y, &accel_off_z);
    bmi160_set_foc_gyro_enable(0x01, &gyro_off_x, &gyro_off_y, &gyro_off_z);

    System_printf("Sensors initialized!\n");
    System_flush();

    snprintf(response,100, "\fInitialization finished\r\nCommand line:\r\n\0");
    UART_write(uart, response, strlen(response));
    counter=0;

    while (1) {

        snprintf(response,100, "\r\nNew Command:\0");
        UART_write(uart, response, strlen(response));

        while (counter<9){
            UART_read(uart, &input, 1);
            command[counter]=input;
            counter++;
        }
        command[9]='\0';

        System_printf("received %s\n",command);
        System_flush();
        counter=0;

        if (!strcmp(command,"comm_gyro")){
            meas_type=GYROSCOPE;
            Semaphore_post(sensor_sem);
        }

        else if (!strcmp(command,"comm_acce")){
            meas_type=ACCELEROMETER;
            Semaphore_post(sensor_sem);
        }

        else if (!strcmp(command,"comm_magn")){
            meas_type=MAGNETOMETER;
            Semaphore_post(sensor_sem);
        }

        else if (!strcmp(command,"comm_led0")){
           Semaphore_post(led0_sem);
        }

        else if (!strcmp(command,"comm_led1")){
            Semaphore_post(led1_sem);
        }

        else{
            snprintf(response,100,"\r\nCommand not understood\r\n\0");
            UART_write(uart,response,strlen(response));
            System_printf(response);
            System_flush();
        }

        Task_sleep(100);
    }
}

void sensorFxn(UArg arg0, UArg arg1)
{
    while(1) {
        //Accel and Gyro update frequency
        Semaphore_pend(sensor_sem,BIOS_WAIT_FOREVER);
        bmi160_read_gyro_xyz(&s_gyroXYZ);
        bmi160_read_accel_xyz(&s_accelXYZ);
        bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
        switch(meas_type){
            case GYROSCOPE:
                snprintf(response,100,"\r\nGyro: %d (X), %d (Y), %d (Z)\n\r\0",s_gyroXYZ.x, -1*s_gyroXYZ.y, -1*s_gyroXYZ.z);
                UART_write(uart, response, strlen(response));
                System_printf(response);
                System_flush();
                break;

            case ACCELEROMETER:
                snprintf(response,100,"\r\nAccel: %d (X), %d (Y), %d (Z)\n\r\0",s_accelXYZ.x, -1*s_accelXYZ.y, -1*s_accelXYZ.z);
                UART_write(uart, response, strlen(response));
                System_printf(response);
                System_flush();
                break;

            case MAGNETOMETER:
                snprintf(response,100,"\r\nMag: %d (X), %d (Y), %d (Z)\n\r\0",s_magcompXYZ.x, s_magcompXYZ.y, s_magcompXYZ.z);
                UART_write(uart, response, strlen(response));
                System_printf(response);
                System_flush();
                break;
        }
   }
}

void LEDfxn(UArg arg0, UArg arg1){
    while(1){
        Semaphore_pend((Semaphore_Handle)arg1,BIOS_WAIT_FOREVER);
        snprintf(response,100,"\r\nToggling success!\r\n\0");
        UART_write(uart,response,strlen(response));
        GPIO_toggle(arg0);
        System_printf(response);
        System_flush();
    }
}

/* ======== main ========*/
int main(void){

    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initUART();

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    /*Semaphore constructions*/
    led0_sem=Semaphore_create(0,NULL,NULL);
    led1_sem=Semaphore_create(0,NULL,NULL);
    sensor_sem=Semaphore_create(0,NULL,NULL);

    /*Sensor Task*/
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = SENSOR_STACK;
    taskParams.instance->name = "Sensor";
    taskParams.priority=2;
    sensor_handle=Task_create(sensorFxn,&taskParams,NULL);

    /*Mission Task*/
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = MISSION_STACK;
    taskParams.instance->name = "UART";
    taskParams.priority=1;
    uart_handle=Task_create(MissionFxn,&taskParams,NULL);

    /*LED Tasks*/
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = LED0_STACK;
    taskParams.instance->name = "Led0";
    taskParams.priority=2;
    taskParams.arg0=Board_LED0;
    taskParams.arg1=(UArg)led0_sem;
    led0_handle=Task_create(LEDfxn,&taskParams,NULL);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = LED1_STACK;
    taskParams.instance->name = "Led1";
    taskParams.priority=2;
    taskParams.arg0=Board_LED1;
    taskParams.arg1=(UArg)led1_sem;
    led0_handle=Task_create(LEDfxn,&taskParams,NULL);

    System_flush();
    /* Start BIOS */
    BIOS_start();

    return (0);
}
