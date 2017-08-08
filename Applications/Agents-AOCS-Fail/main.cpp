#include "app.h"

/*Agent object building, agent stack declaration and Agent_Platform object building*/
Agent_Stack UARTSTACK[2048], SENSORSTACK[2048], KALMANSTACK[4096];
Agent UART_Agent("UART Agent", 2,UARTSTACK,2048);
Agent Sensor_Agent("Sensor Control Agent",1,SENSORSTACK,2048);
Agent Kalman_Agent("Kalman Agent",2,KALMANSTACK,4096);
Agent_Platform AP("AOCS");
/*Callback functions for the button*/

void gpioButtonFxn0(unsigned int index){

    button_pressed=true;
//    GPIO_toggle(Board_LED0);
//    System_printf("something\n");
//    System_flush();
}

/*  ======== main ========*/
int main(void){
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initUART();

    /*Set button interrupt*/
    GPIO_setCallback(Board_BUTTON0, gpioButtonFxn0);
    GPIO_enableInt(Board_BUTTON0);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

  //  BMI=BMI_Agent.create(matrix_func, 2);
    AP.agent_init(UART_Agent, uartFxn);
    AP.agent_init(Sensor_Agent, sensor);
    AP.agent_init(Kalman_Agent, kalman);
    Kalman_AID=Kalman_Agent.AID();
    Sensor_AID=Sensor_Agent.AID();
    UART_AID=UART_Agent.AID();

    AP.boot();
    System_flush();

    /*BIOS_start*/
    BIOS_start();

    return (0);
}
