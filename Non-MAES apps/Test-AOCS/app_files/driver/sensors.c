#include "app.h"
void sensor(UArg arg0, UArg arg1){
    //BMI_on = true;
    Uint32 timeInit,timeSleep;
    GateMutexPri_Params_init(&gateParams);
    i2cGate = GateMutexPri_create(&gateParams, NULL);
    /* Initialize I2C */
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

//    timeInit=0;

    do{
        timeInit=Clock_getTicks();

        //Read Accel and Gyro values
        gatekey = GateMutexPri_enter(i2cGate);
        bmi160_read_accel_xyz(&s_accelXYZ);
        bmi160_read_gyro_xyz(&s_gyroXYZ);
        bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
        GateMutexPri_leave(i2cGate, gatekey);

        /*Sending to UART port*/
        //  Load_update();
        Semaphore_post(Kalman_sem);
        timeSleep=Clock_getTicks()-timeInit;
        Task_sleep(100-timeSleep);

    }while(1);
}
/***********************************************************
  Funtion:
   Task handling the formatting of JSON string to be sent over
   UART to the GUI.
   Multiplying -1 to Y and Z axis of BMI160 sensor to match
   BMI150's axes
 ***********************************************************/
void uartFxn(UArg arg0, UArg arg1){
    while (1) {
        Semaphore_pend(UART_sem, BIOS_WAIT_FOREVER);
       // Log_write1(UIABenchmark_start, (xdc_IArg)"running");
        snprintf(stringBuffer, 295,
                "{\"gyro\":{\"x\":%d,\"y\":%d,\"z\":%d},"
                "\"accel\":{\"x\":%d,\"y\":%d,\"z\":%d},"
                "\"mag\":{\"x\":%d,\"y\":%d,\"z\":%d},"
                "\"quaternions\":{\"q1\":%.14f,\"q2\":%.14f,\"q3\":%.14f,\"q4\":%.14f}}\n",
                s_gyroXYZ.x, -1*s_gyroXYZ.y, -1*s_gyroXYZ.z,
                s_accelXYZ.x, -1*s_accelXYZ.y, -1*s_accelXYZ.z,
                s_magcompXYZ.x, s_magcompXYZ.y, s_magcompXYZ.z,
                x.pData[0],x.pData[1],x.pData[2],x.pData[3]);

        gatekey = GateMutexPri_enter(i2cGate);
        UART_write(uart, stringBuffer, strlen(stringBuffer));
        GateMutexPri_leave(i2cGate, gatekey);
      //  Log_write1(UIABenchmark_stop, (xdc_IArg)"running");
    }
}


