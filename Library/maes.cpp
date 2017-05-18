/*
 * maes.cpp
 *
 *  Created on: 14 May 2017
 *      Author: ckc00
 */

#include "maes.h"

namespace MAES{

    /*********************************************************************************************
    * Class: Agent_Build
    * Function: Agent_Build constructor
    * Comments: task_stack_size: set to default value 512
    *           msg_queue_size: set to default value 5
    *           msg_size: set to predefined MsgObj size
    **********************************************************************************************/
    Agent_Build::Agent_Build(Task_Handle t,
                             String name,
                             Mailbox_Handle m,
                             int pri,
                             Task_FuncPtr b){ // Constructor{ // Constructor
        Agent_Msg MsgObj;

        /*Mailbox init*/
        mailbox_handle=m;
        msg_size=sizeof(MsgObj);
        msg_queue_size=5;

        /*Task init*/
        task_handle=t;
        agent_name=name;
        priority=pri;
        behaviour=b;
        task_stack_size=512;
        task_stack=new char[task_stack_size];
        Task_Params_init(&taskParams);
        taskParams.stack= &task_stack;
        taskParams.stackSize =task_stack_size;
        taskParams.priority = priority;
        taskParams.instance->name=agent_name;
        taskParams.env=(xdc_Ptr) mailbox_handle;//Check if this works
    }

    /*********************************************************************************************
    * Class: Agent_Build
    * Function: Agent_Build constructor
    * Comments: For user who wants to set own stack, msg, queue size values
    **********************************************************************************************/
    Agent_Build::Agent_Build(Task_Handle t,
                             String name,
                             Mailbox_Handle m,
                             int pri,
                             Task_FuncPtr b,
                             int taskStackSize,
                             int msgSize,
                             int msgQueueSize){ // Constructor
         /*Mailbox init*/
        mailbox_handle=m;
        msg_size=msgSize;
        msg_queue_size=msgQueueSize;

        //Mailbox_Params_init(&mbxParams);


        /*Task init*/
        task_handle=t;
        agent_name=name;
        priority=pri;
        behaviour=b;
        task_stack_size=taskStackSize;
        task_stack=new char[task_stack_size];
        Task_Params_init(&taskParams);
        taskParams.stack= &task_stack;
        taskParams.stackSize =task_stack_size;
        taskParams.priority = priority;
        taskParams.instance->name=agent_name;
        taskParams.env=(xdc_Ptr) mailbox_handle;//Check if this works
    }


    /*********************************************************************************************
     * Class: Agent_Build
     * Function: void create_agent()
     * Return type: NULL
     * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
     *           embedded in task's handle env variable.
    *********************************************************************************************/
    void Agent_Build::create_agent(){

        System_printf("test");
        /*Creating task*/
      //   mailbox_handle= Mailbox_create(msg_size,msg_queue_size,&mbxParams,NULL);
         task_handle = Task_create(behaviour, &taskParams, NULL);
    }
     /*********************************************************************************************
     * Class: Agent_AMS
     * Function: get_AID(Task_Handle t);
     * Return type: String
     * Comments: When using this function set in .cfg file: Task.common$.namedInstance = true
     ********************************************************************************************/
     String Agent_AMS::get_AID(Task_Handle t){
        return Task_Handle_name(t);
    }


};


