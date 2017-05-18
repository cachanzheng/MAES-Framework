/*
 * maes.cpp
 *
 *  Created on: 14 May 2017
 *      Author: Carmen Chan-Zheng
 */

#include "maes.h"

namespace MAES{
/*********************************************************************************************
*
*                                  Class: Agent_Build
*
**********************************************************************************************

*********************************************************************************************
* Class: Agent_Build
* Function: Agent_Build constructor
* Comments: task_stack_size: set to default value 512
*           msg_queue_size: set to default value 5
*           msg_size: set to predefined MsgObj size
**********************************************************************************************/
    Agent_Build::Agent_Build(String name,
                             int pri,
                             Task_FuncPtr b){

        Agent_Msg MsgObj;

        /*Mailbox init*/
        msg_size=12;  //String: 4, Int: 4, String: 4
        msg_queue_size=5;

        /*Task init*/
        agent_name=name;
        priority=pri;
        behaviour=b;
        task_stack_size=512;
        task_stack=new char[task_stack_size];
    }

/*********************************************************************************************
 * Class: Agent_Build
 * Function: void create_agent()
 * Return type: NULL
 * Comments: Create the task and mailbox associated with the agent. The mailbox handle is
 *           embedded in task's handle env variable.
*********************************************************************************************/
    void Agent_Build::create_agent(){


        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        mailbox_handle= Mailbox_create(msg_size,msg_queue_size,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize =task_stack_size;
        taskParams.priority = priority;
        taskParams.instance->name=agent_name;
        taskParams.env=(xdc_Ptr)mailbox_handle;
        task_handle = Task_create(behaviour, &taskParams, NULL);
    }

/*********************************************************************************************
 * Class: Agent_Build
 * Function: create_agent()
 * Return type: String
 * Comments: Returns agent's name
*********************************************************************************************/
    String Agent_Build::get_name(){
        return agent_name;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function:  get_task_handle(){
* Return type: Task Handle
* Comments: Returns agent's task handle
*********************************************************************************************/
    Task_Handle Agent_Build::get_task_handle(){
        return task_handle;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: get_mailbox_handle()
* Return type: Mailbox Handle
* Comments: Returns agent's mailbox handle
*********************************************************************************************/
    Mailbox_Handle Agent_Build::get_mailbox_handle(){
        return mailbox_handle;
    }

/*********************************************************************************************
* Class: Agent_Build
* Function: get_prio()
* Return type: int
* Comments: Returns agent's priority
*********************************************************************************************/
    int Agent_Build::get_prio(){
        return priority;
    }

/*********************************************************************************************
*
*                                  Class: Agent_Msg
*
*********************************************************************************************

*********************************************************************************************
* Class: Agent_Msg
* Function: Agent_Msg Constructor
* Comment: Construct Msg Object. Msg object shall be created in the task function. Therefore,
* the mailbox handle contained in task environment variable is assigned directly to that
* msg object
**********************************************************************************************/
   Agent_Msg::Agent_Msg(){
      mailbox_handle=(Mailbox_Handle) Task_getEnv(Task_self());
      MsgObj.sender=Task_Handle_name(Task_self());
      clear_all_receiver();
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: send_msg()
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox
**********************************************************************************************/
    bool Agent_Msg::send(Mailbox_Handle m){ //To do: list of senders
        return Mailbox_post(m, (xdc_Ptr)&MsgObj, BIOS_NO_WAIT);

    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send_msg_()
* Return type: Boolean. TRUE if all msgs are sent successfully to all receivers
*                       FALSE if there were an error.
* Comment: Iterate over the list. If there is an error for any receiver, error will turn true.
*          Also, return true if list of receiver is NULL.
**********************************************************************************************/
    bool Agent_Msg::send(){ //To do: list of senders
        int i=0;
        bool error=false;

        while(i<8){
            if (receivers[i]==NULL) break;
            else {
                if(Mailbox_post(receivers[i], (xdc_Ptr)&MsgObj, BIOS_NO_WAIT)) error=true;
            }
            i++;
        }

        if (i==0) error=true;

        return error;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: receive_msg(Uint32 timeout)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Receiving msg in its queue. Block call
**********************************************************************************************/
    bool Agent_Msg::receive(Uint32 timeout){

        return Mailbox_pend(mailbox_handle, (xdc_Ptr) &MsgObj, timeout);
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Mailbox_Handle m)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers
**********************************************************************************************/
    bool Agent_Msg::add_receiver(Mailbox_Handle m){
        int i=0;

        if (m==NULL){
            System_printf("Handle is null \n");
            System_flush();
            return false;
        }

        else{
            while (i<RECEIVER_LIST_SIZE){
                if (receivers[i]==m){
                    System_printf("Receiver already in the list \n");
                    System_flush();
                    break;
                }
                if (receivers[i]==NULL){
                    receivers[i]=m;
                    break;
                }
                i++;
            }
        }

        if(i==8){
            System_printf("Receivers List is full \n");
            System_flush();
            return false;
        }
        else return true;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Mailbox_Handle m)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
    bool Agent_Msg::remove_receiver(Mailbox_Handle m){

        int i=0;

        while(i<RECEIVER_LIST_SIZE){
            if(receivers[i]==m){
                while(i<RECEIVER_LIST_SIZE-1){
                    receivers[i]=receivers[i+1];
                    i++;
                }
                receivers[RECEIVER_LIST_SIZE-1]=NULL;
                break;
            }
            i++;
        }

        if (i==8) return false;
        else return true;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: clear_all_receiver();
* Return type: Boolean. True if receiver is removed successfully.
* Comment: Remove receiver in list of receivers
**********************************************************************************************/
    void Agent_Msg::clear_all_receiver(){
        int i=0;
        while (i<RECEIVER_LIST_SIZE){
            receivers[i]=NULL;
            i++;
        }
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_type(int type)
* Return type: NULL
* Comment: Set message type according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_type(int msg_type){
        MsgObj.type=msg_type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_body(String body)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_body(String msg_body){
        MsgObj.body=msg_body;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_type()
* Return type: int
* Comment: Get message type
**********************************************************************************************/
    int Agent_Msg::get_msg_type(){
        return MsgObj.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_body()
* Return type: String
* Comment: Get message body
**********************************************************************************************/
    String Agent_Msg::get_msg_body(){
        return MsgObj.body;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: String
* Comment: Get sender name
**********************************************************************************************/
    String Agent_Msg::get_sender(){
        return MsgObj.sender;
    }


/*********************************************************************************************
*
*                                  Class: Agent_AMS
*
********************************************************************************************

*********************************************************************************************
* Class: Agent_AMS
* Function: get_AID(Task_Handle t);
* Return type: String
* Comments: When using this function set in .cfg file: Task.common$.namedInstance = true
********************************************************************************************/
    String Agent_AMS::get_AID(Task_Handle t){
        return Task_Handle_name(t);
    }


};


