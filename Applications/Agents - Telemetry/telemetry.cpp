#include <xdc/std.h>
#include <xdc/runtime/System.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Board Header file */
#include "Board.h"


/*Including Agents*/
#include "maes.h"
using namespace MAES;

#define Agent_Stack_Size   2048

UART_Handle uart;
UART_Params uartParams;

Agent_Stack currentstack[Agent_Stack_Size];
Agent_Stack voltagestack[Agent_Stack_Size];
Agent_Stack temperaturestack[Agent_Stack_Size];
Agent_Stack measurementstack[Agent_Stack_Size];

Agent logger_current("Current Agent",4,currentstack,Agent_Stack_Size);
Agent logger_voltage("Voltage Agent",3,voltagestack,Agent_Stack_Size);
Agent logger_temperature("Temperature Agent",2,temperaturestack,Agent_Stack_Size);
Agent measurement("Gen Agent",5,measurementstack,Agent_Stack_Size);

/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");


typedef enum meas_type{
    CURRENT,
    VOLTAGE,
    TEMPERATURE
}meas_type;

typedef struct logger_info{
    meas_type type;
    int rate; //in ms.
}logger_info;

logger_info log_current, log_voltage,log_temperature;


class loggerBehaviour: public CyclicBehaviour{
public:
    logger_info *info;
    void setup(){
        info=(logger_info *)arg0;
    }

    void action(){
        msg.set_msg_content((String)info->type);
        msg.send(measurement.AID(), BIOS_WAIT_FOREVER);
        msg.receive(BIOS_WAIT_FOREVER);

        /**Logging*/
        UART_write(uart, msg.get_msg_content(), strlen(msg.get_msg_content()));
        System_printf("%s\n",msg.get_msg_content());
        System_flush();
        AP.agent_wait(info->rate); // Sleep the rest until next period.
    }
};
void logger(UArg arg0, UArg arg1){
    loggerBehaviour b;
    b.arg0=arg0;
    b.execute();

};

class genBehaviour:public CyclicBehaviour{
public:
    float min,max,value;
    char response[50];

    void setup(){
        /* Create a UART with data processing off. */
      UART_Params_init(&uartParams);
      uartParams.writeDataMode = UART_DATA_BINARY;
      uartParams.baudRate = 115200;
      uart = UART_open(Board_UART0, &uartParams);
      if (uart == NULL) System_abort("Error opening the UART");
    }
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        switch((int)msg.get_msg_content()){
            case CURRENT:
                min=0.1; //0mA
                max=1000; //1000mA
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(response,50,"\r\nCurrent measurement: %f\r\n",value);

                break;
            case VOLTAGE:
                min=0.5; //0V
                max=3.3;//3.3V
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(response,50,"\r\nVoltage measurement: %f\r\n",value);

                break;
            case TEMPERATURE:
                min=30; //0 C;
                max=100;//100C;
                value=min + rand() / (RAND_MAX / (max - min + 1) + 1);
                snprintf(response,50,"\r\nTemperature measurement: %f\r\n",value);
          break;
            default:
                snprintf(response,50,"\r\nNot understood");
                break;
        }

        msg.set_msg_content(response);
        msg.send(msg.get_sender(), BIOS_WAIT_FOREVER);
    }
};

void gen_meas(UArg arg0, UArg arg1){
    genBehaviour b;
    b.execute();
};
/*
 *  ======== main ========
 */
int main(void)
{

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();
    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    /*Construct loggers*/
    log_current.rate=500;
    log_current.type=CURRENT;

    log_voltage.rate=1000;
    log_voltage.type=VOLTAGE;

    log_temperature.rate=2000;
    log_temperature.type=TEMPERATURE;

    /*Platform init*/
    AP.agent_init(logger_current, logger, (UArg)&log_current, 0);
    AP.agent_init(logger_voltage, logger, (UArg)&log_voltage, 0);
    AP.agent_init(logger_temperature, logger, (UArg)&log_temperature, 0);
    AP.agent_init(measurement,gen_meas);
    AP.boot();


    /* Start BIOS */
    BIOS_start();

    return (0);
}

