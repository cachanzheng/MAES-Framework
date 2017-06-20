#include "app.h"

/*Agent object building and Agent_Platform object building*/
Agent UART_Agent("UART Agent");
Agent Sensor_Agent("Sensor Control Agent");
Agent Kalman_Agent("Kalman Agent");
Agent_Platform AP("AOCS");

/*  ======== main ========*/
int main(void){
	/* Call board init functions */
	Board_initGeneral();
	Board_initGPIO();
	Board_initI2C();
	Board_initUART();

	/* Turn on user LED */
	GPIO_write(Board_LED0, Board_LED_ON);

  //  BMI=BMI_Agent.create(matrix_func, 2);
	UART_AID=UART_Agent.create(uartFxn,2);
	Sensor_AID=Sensor_Agent.create(sensor, 1);
	Kalman_AID=Kalman_Agent.create(kalman,4096,8,2);

	AP.init();
	System_flush();

	/*BIOS_start*/
	BIOS_start();

	return (0);
}
