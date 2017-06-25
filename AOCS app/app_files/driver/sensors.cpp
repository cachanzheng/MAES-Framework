#include "app.h"
/***********************************************************
  Funtion:
   - Main thread handler for system. Determines when to update
   sensor values using a counter, what gesture should be displayed
   in the GUI, and when to send over UART.
   - Task handling the reading of the BMI160 sensor.
   Multiplying -1 to Y and Z axis of BMI160 sensor to match
   BMI150's axes
***********************************************************/
class sensor_control:public CyclicBehaviour{
    Uint32 timeBMI;
    void setup(){
       //BMI_on = true;
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

       timeBMI=0;
    }

    void action(){
        /*Accel and Gyro update frequency
         * 100:100000us, 1000 is 1 seconds 1000us*1000
         * (since system tick is 1000us)
         */
        if ((abs(Clock_getTicks()-timeBMI)>=100)){
          timeBMI=Clock_getTicks();
          //Read Accel and Gyro values
          gatekey = GateMutexPri_enter(i2cGate);
          bmi160_read_accel_xyz(&s_accelXYZ);
          bmi160_read_gyro_xyz(&s_gyroXYZ);
          bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
          GateMutexPri_leave(i2cGate, gatekey);

          /*Sending to UART port*/
          msg.send(Kalman_AID);
        }
    }
};
/*Class wrapper*/
void sensor(UArg arg0, UArg arg1){
    sensor_control behaviour;
    behaviour.execute();
}
/***********************************************************
  Funtion:
   Task handling the formatting of JSON string to be sent over
   UART to the GUI.
   Multiplying -1 to Y and Z axis of BMI160 sensor to match
   BMI150's axes
 ***********************************************************/
void uartFxn(UArg arg0, UArg arg1){
    Agent_Msg msg;
    while (1) {
        msg.receive(BIOS_WAIT_FOREVER);

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
    }
}


