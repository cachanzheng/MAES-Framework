/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "Board.h"

/* Include Agent file*/
#include "maes.h"
#include <stdlib.h>
#include <stdio.h>
using namespace MAES;

Agent_Stack Agent1[1024];
Agent_Stack Agent2[1024];
Agent_Stack Agent3[1024];
Agent Reading("Agent 1", 2, Agent1,1024);
Agent Reading2("Agent 2", 2, Agent2,1024);
Agent Writing("Writing Agent",1,Agent3,1024);


class WritingBehaviour:public CyclicBehaviour{
public:
    void setup(){
       // msg.add_receiver(Reading.AID());
        //msg.add_receiver(Reading2.AID());
        System_printf("size %d\n");
    }

    void action(){
        Task_sleep(1000);
        msg.send(Reading.AID(),BIOS_WAIT_FOREVER);
        msg.send(Reading2.AID(),BIOS_WAIT_FOREVER);

    }
};
void function(UArg arg0, UArg arg1){
    WritingBehaviour b;
    b.execute();
};


class ReadingBehaviour:public CyclicBehaviour{
public:
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Agent1\n");
        System_flush();
        GPIO_toggle(Board_LED0);
    }
};
void reading(UArg arg0, UArg arg1){
    ReadingBehaviour b;
    b.execute();
};

class ReadingBehaviour2:public CyclicBehaviour{
public:
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Agent2\n");
        System_flush();
        GPIO_toggle(Board_LED1);
    }
};
void reading2(UArg arg0, UArg arg1){
    ReadingBehaviour2 b;
    b.execute();
};

/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");


/*  ======== main ========
 */
int main()
{

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    AP.agent_init(Reading, reading);
    AP.agent_init(Writing, function);
    AP.agent_init(Reading2, reading2);
    AP.boot();

    BIOS_start();
    return (0);
}
