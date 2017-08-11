#include "app.h"

/*Agent object building, agent stack declaration and Agent_Platform object building*/
char UARTSTACK[2048], SENSORSTACK[2048], KALMANSTACK[4096];



/*  ======== main ========*/
int main(void){
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initUART();

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    Kalman_sem=Semaphore_create(0, NULL, NULL);
    UART_sem=Semaphore_create(0, NULL, NULL);

    /*Construct UART*/
    Task_Params_init(&taskParams);
    taskParams.stack = UARTSTACK;
    taskParams.stackSize = 2048;
    taskParams.priority = 2;
    UART_AID = Task_create(uartFxn, &taskParams, NULL);

    /*Construct SENSOR*/
    Task_Params_init(&taskParams);
    taskParams.stack = SENSORSTACK;
    taskParams.stackSize = 2048;
    taskParams.priority = 1;
    Sensor_AID = Task_create(sensor, &taskParams, NULL);

    /*Construct KALMAN*/
    Task_Params_init(&taskParams);
    taskParams.stack = KALMANSTACK;
    taskParams.stackSize = 4096;
    taskParams.priority = 2;
    Kalman_AID = Task_create(kalman, &taskParams, NULL);

    System_flush();

    /*BIOS_start*/
    BIOS_start();

    return (0);
}
