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


/*Callback functions for the button*/
bool button_pressed;
void gpioButtonFxn0(unsigned int index){
    button_pressed=true;
}

/*Constructing platform and AID handle*/
Agent_Platform AP("Texas Instruments");

///*Creating Agent organization group*/
//ORG_TYPE type =HIERARCHY;
Agent_Organization org(HIERARCHY);


/*Constructing Agents*/
Agent_Stack FDIRStack[1024], Worker1Stack[1024], Worker2Stack[1024], Worker3Stack[1024],SupervisorStack[2048],PrintStack[1024];
Agent FDIRAgent("FDIR Agent",3, FDIRStack,1024);
Agent Worker1("Worker1",2,Worker1Stack,1024);
Agent Worker2("Worker2",2,Worker2Stack,1024);
Agent Worker3("Worker3",2,Worker3Stack,1024);
Agent Supervisor("Supervisor",3, SupervisorStack,2048);
Agent Print("Print",3,PrintStack,1024);
class FDIRbehaviour:public CyclicBehaviour{
public:
    int failure_count;
    void setup(){
        System_printf("Initializing\n");
        System_flush();
        failure_count=0;
        button_pressed=false;
    }

    void action(){
        AP.agent_wait(500);
    }

    bool failure_detection(){
        if (button_pressed){
            failure_count++;
            System_printf("Failure detected number %d \n", failure_count);
            System_flush();
            button_pressed=false;
            if (failure_count==5) msg.restart();
            return true;

        }
        else return false;
    }

    void failure_identification(){
        System_printf("Identifying failure\n");
        System_flush();
    }
    void failure_recovery(){
        System_printf("Recovering from failure\n");
        System_flush();
    }
};
void wrapperFDIR(UArg arg0, UArg arg1){
    FDIRbehaviour b;
    b.execute();
};


class SupervisorBehaviour:public CyclicBehaviour{
    int read_value;
    Agent_info info;
    char body_msg[100];


    void setup(){
        org.create();
        org.set_admin(Supervisor.AID());
        org.set_moderator(Supervisor.AID());
        org.add_agent(Worker1.AID());
        org.add_agent(Worker2.AID());
        org.add_agent(Worker3.AID());
    }
    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        read_value = (int) msg.get_msg_content();

        if (read_value>70){
            info=AP.get_Agent_description(msg.get_sender());

            snprintf(body_msg, 100, "Agent Worker %s detected temperature %d \n", info.agent_name, read_value);
            msg.set_msg_content(body_msg);
            msg.send(Print.AID(),BIOS_WAIT_FOREVER);
        }
    }
};

void wrapperSupervisor(UArg arg0, UArg arg1){
    SupervisorBehaviour b;
    b.execute();
}


class WorkerBehaviour:public CyclicBehaviour{
public:
    int random;

    void action(){
        random=rand()%100;
        msg.set_msg_content((String) random);
        msg.send(Supervisor.AID(),BIOS_WAIT_FOREVER);
        AP.agent_wait(1000);
    }
};

void wrapperWorker(UArg arg0, UArg arg1){
    WorkerBehaviour b;
    b.execute();
}

class PrintBehaviour:public CyclicBehaviour{

    void action(){
        msg.receive(BIOS_WAIT_FOREVER);
        System_printf("Warning: %s", msg.get_msg_content());
        System_flush();

    }

};

void wrapperPrint(UArg arg0, UArg arg1){
    PrintBehaviour b;
    b.execute();
}

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

    /*Set button interrupt*/
    GPIO_setCallback(Board_BUTTON0, gpioButtonFxn0);
    GPIO_enableInt(Board_BUTTON0);

    /*Set Agents*/
    AP.agent_init(FDIRAgent, wrapperFDIR);
//    AP.agent_init(Supervisor, wrapperSupervisor);
  //  AP.agent_init(Worker1, wrapperWorker);
    //AP.agent_init(Worker2, wrapperWorker);
    //AP.agent_init(Worker3, wrapperWorker);
    //AP.agent_init(Print, wrapperPrint);
    AP.boot();

    BIOS_start();
    return (0);
}
