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
/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");



/*Callback functions for the button*/
bool button_pressed;
void gpioButtonFxn0(unsigned int index){

    button_pressed=true;
}


class WritingBehaviour:public CyclicBehaviour{
public:
    void setup(){
//        System_printf("Initializing\n");
//        System_flush();
        msg.add_receiver(Reading.AID());
        msg.add_receiver(Reading2.AID());
    }

    void action(){
        AP.agent_wait(100);
   //     System_printf("Active Agent: Writing Agent\n");
//        init=Timestamp_get32();
        msg.send();
    }

    bool failure_detection(){
           if (button_pressed){
               button_pressed=false;
               return true;

           }
           else return false;
       }

   void failure_identification(){
       init=Timestamp_get32();
//       System_printf("Identifying failure\n");
//       System_flush();
   }
   void failure_recovery(){
//       System_printf("Recovering from failure\n");
//       System_flush();
       msg.restart();
   }
};
void writing(UArg arg0, UArg arg1){
    WritingBehaviour b;
    b.execute();
};


class ReadingBehaviour:public CyclicBehaviour{
public:
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
//        stop=Timestamp_get32();
//        System_printf("Difference %d\n", stop-init);
//        System_flush();
//        System_printf("Active Agent: Reading Agent 1. Toggling LED0\n");
//        System_flush();
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
//        System_printf("Active Agent: Reading Agent 2. Toggling LED1\n");
//        System_flush();
        GPIO_toggle(Board_LED1);
    }
};
void reading2(UArg arg0, UArg arg1){
    ReadingBehaviour2 b;
    b.execute();
};




/*  ======== main ========
 */
int main()
{

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    GPIO_write(Board_LED0, Board_LED_ON);

    /*Set button interrupt*/
    GPIO_setCallback(Board_BUTTON0, gpioButtonFxn0);
    GPIO_enableInt(Board_BUTTON0);

    /* SysMin will only print to the console when you call flush or exit */
    System_printf("Blinking Led Example with agents \n");
    System_flush();

    AP.agent_init(Reading, reading);
    AP.agent_init(Writing, writing);
    AP.agent_init(Reading2, reading2);
    AP.boot();


    BIOS_start();
    return (0);
}
