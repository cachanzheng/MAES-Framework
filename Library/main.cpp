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

/*Construction functions*/
void writing(UArg arg0, UArg arg1);
void reading(UArg arg0, UArg arg1);
void reading2(UArg arg0, UArg arg1);
void function(UArg arg0, UArg arg1);


Agent_AID WritingAID,ReadingAID,Reading2AID,test123;
Agent_Stack Agent1[2048];
Agent_Stack Agent2[2048];
Agent_Stack Agent3[4096];
Agent_Stack Agent4[1024];
Agent Reading("Agent 1", 2, Agent1,2048);
Agent Reading2("Agent 2", 2, Agent2,2048);
Agent Writing("Writing Agent",1,Agent3,4096);
Agent test("New Agent",2,Agent4,1024);

class behaviours:public CyclicBehaviour{
public:
    void setup(){
        msg.add_receiver(ReadingAID);
        msg.add_receiver(Reading2AID);
    }

    void action(){

       // msg.set_msg_string(55);
      //  System_printf("error %x\n", msg.send(ReadingAID));
       msg.send();
        System_flush();
        Task_sleep(500);

    }
};
void function(UArg arg0, UArg arg1){
    behaviours b;
    b.execute();
};

/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");
Agent_Organization Org1(HIERARCHY);

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

    AP.agent_init(Reading, reading, ReadingAID);
    AP.agent_init(Writing, function, WritingAID);
    AP.agent_init(Reading2, reading2, Reading2AID);
  //  AP.agent_init(test, reading2, test123);
    AP.boot();

    BIOS_start();
    return (0);
}

void reading(UArg arg0, UArg arg1)
{
    Agent_Msg msg;
    int i=0;

    while(1) {

        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Agent1\n");
        System_flush();
        AP.agent_wait(500);
        GPIO_toggle(Board_LED0);
        i++;
    }
}

void reading2(UArg arg0, UArg arg1)
{
    Agent_Msg msg;
    int i=0;
    Org1.create();
    System_printf("Adding %x\n", Org1.add_agent(ReadingAID));
    System_printf("Adding %x\n", Org1.add_agent(WritingAID));
    System_printf("Change %x\n", Org1.set_admin(Reading2AID));
    System_printf("Change %x\n", Org1.set_moderator(Reading2AID));
    System_flush();
    while(1) {
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Agent2\n");
        System_flush();
        GPIO_toggle(Board_LED1);

        if (i==5) {
                   Org1.destroy();


       }
        i++;
     }
}

