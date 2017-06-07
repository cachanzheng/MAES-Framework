#include "maes.h"
namespace MAES
{
/*********************************************************************************************
*
*                                  Class: Agent_Msg
*
********************************************************************************************
*********************************************************************************************
* Class: Agent_Msg
* Function:Agent_Msg Constructor
* Comment: Construct Agent_Msg Object.
*          Msg object shall be created in the task function, therefore,
*          the Agent_msg object is assigned to the handle of the calling task.
*          The object contains information of the task handle, mailbox and the name
**********************************************************************************************/
   Agent_Msg::Agent_Msg(){
      Agent_info *description=(Agent_info*)Task_getEnv(Task_self());
      self_handle=Task_self();
      self_mailbox=description->mailbox_handle;
      clear_all_receiver();
      next_available=0;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: private bool isRegistered(Agent aid);
* Return type: Boolean
* Comment: if returns false, sender or receiver is not registered in the same platform
**********************************************************************************************/
  bool Agent_Msg::isRegistered(Agent aid){
      Agent_info *description_receiver=(Agent_info *)Task_getEnv(aid);
      Agent_info *description_sender=(Agent_info *)Task_getEnv(self_handle);

      if(description_receiver->AP==description_sender->AP) return true;
      else return false;
}

/*********************************************************************************************
* Class: Agent_Msg
* Function: private get_mailbox(Agent aid)
* Return type: Mailbox_Handle
**********************************************************************************************/
    Mailbox_Handle Agent_Msg::get_mailbox(Agent aid){
        Agent_info * description;
        description = (Agent_info*) Task_getEnv(aid);
        return description->mailbox_handle;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Agent aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
   int Agent_Msg::add_receiver(Agent aid_receiver){

        if(isRegistered(aid_receiver)){
           if (aid_receiver==NULL) return HANDLE_NULL;

           if(next_available<MAX_RECEIVERS){
               receivers[next_available]=aid_receiver;
               next_available++;
               return NO_ERROR;
           }

           else return LIST_FULL;

       }
       else return NOT_FOUND;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: remove_receiver(Agent aid)
* Return type: Boolean. True if receiver is removed successfully. False if it is not encountered
* Comment: Remove receiver in list of receivers. It searches inside of the list, when found,
* the rest of the list is shifted to the right and the receiver is removed.
**********************************************************************************************/
   int Agent_Msg::remove_receiver(Agent aid){

        int i=0;
        while(i<MAX_RECEIVERS){
            if(receivers[i]==aid){
                while(i<MAX_RECEIVERS-1){
                    receivers[i]=receivers[i+1];
                    i++;
                }
                receivers[MAX_RECEIVERS-1]=NULL;
                next_available--;
                break;
            }
            i++;
        }
        if (i==MAX_RECEIVERS) return NOT_FOUND;
        else return NO_ERROR;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: clear_all_receiver();
* Return type: NULL
* Comment: Clear all receiver in the list
**********************************************************************************************/
    void Agent_Msg::clear_all_receiver(){
        int i=0;
        while (i<MAX_RECEIVERS){
            receivers[i]=NULL;
            i++;
        }
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: refresh_list()
* Return type: NULL
* Comment: Refresh the list with all the registered agents. Remove agent if it is not registered
**********************************************************************************************/
    void Agent_Msg::refresh_list(){
        int i=0;
        while (i<next_available){
            if(!isRegistered(receivers[i]))remove_receiver(receivers[i]);
            i++;
        }
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: receive_msg(Uint32 timeout)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Receiving msg in its queue. Block call. The mailbox is obtained from the
*          task handle of the calling function of this object.
**********************************************************************************************/
    bool Agent_Msg::receive(Uint32 timeout){

        return Mailbox_pend(self_mailbox, (xdc_Ptr) &msg, timeout);
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send(Agent aid)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
**********************************************************************************************/
    int Agent_Msg::send(Agent aid_receiver){
        msg.sender_agent=self_handle;
        msg.target_agent=aid_receiver;

        if(isRegistered(aid_receiver)){
            if(Mailbox_post(get_mailbox(aid_receiver), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
            else return TIMEOUT;
        }
        else return NOT_FOUND;

    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if all msgs are sent successfully to all receivers
*                       FALSE if there were an error.
* Comment: Iterate over the list. If there is an error for any receiver, will return false
*          if there is any error.
**********************************************************************************************/
     bool Agent_Msg::send(){
        int i=0;
        bool no_error=true;
        msg.sender_agent=self_handle;

        while (receivers[i]!=NULL){
            if(isRegistered(receivers[i])){
                msg.target_agent=receivers[i];
                if(!Mailbox_post(get_mailbox(receivers[i]),(xdc_Ptr)&msg, BIOS_NO_WAIT));
                    no_error=false;
                i++;

            }
            else refresh_list();
        }
        return no_error;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_type(int type)
* Return type: NULL
* Comment: Set message type according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_type(int msg_type){
        msg.type=msg_type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_string(String content)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_string(String msg_body){
        msg.content_string=msg_body;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_int(int content)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_int(int content){
        msg.content_int=content;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg();
* Return type: MsgObj
* Comment: Get message
**********************************************************************************************/
    MsgObj *Agent_Msg::get_msg(){
        MsgObj *ptr =&msg;
        return ptr;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_type()
* Return type: int
* Comment: Get message type
**********************************************************************************************/
    int Agent_Msg::get_msg_type(){
        return msg.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_string()
* Return type: String
* Comment: Get string content
**********************************************************************************************/
    String Agent_Msg::get_msg_string(){
        return msg.content_string;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_int()
* Return type: int
* Comment: Get int content
**********************************************************************************************/
    int Agent_Msg::get_msg_int(){
        return msg.content_int;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: Agent
* Comment: Get sender
**********************************************************************************************/
    Agent Agent_Msg::get_sender(){
        return msg.sender_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_target_agent()
* Return type: Agent
* Comment: Get target aid
**********************************************************************************************/
    Agent Agent_Msg::get_target_agent(){
        return msg.target_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: int request_AP(int request, Agent target_agent,int timeout)
* Return type: Int
* Comment: request the Agent Platform to perform a service and wait for response during
*          a time specified by the user
**********************************************************************************************/
    int Agent_Msg::request_AP(int request, Agent target_agent,int timeout){
        Agent AMS;
        Agent_info *temp;

        if (request != BROADCAST && request!=MODIFY){

            /*Setting msg*/
            msg.type=REQUEST;
            msg.content_int=request;
            msg.target_agent=target_agent;
            msg.sender_agent=Task_self();

            /*Getting AP address:
             * 1. Get the Agent info
             * 2. Get the AMS info*/
            temp= (Agent_info*) Task_getEnv(self_handle);
            AMS=temp->AP;

            /*Sending request*/
            Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

            /*Waiting for answer*/
            receive(timeout);

            return msg.type;
        }

        else return REFUSE;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: int request_AP(int request, Agent target_agent,int timeout,int content)
* Return type: Int
* Comment: request the agent to modify priority of target agent
**********************************************************************************************/
    int Agent_Msg::request_AP(int request, Agent target_agent,int timeout, int content){
        Agent AMS;
        Agent_info *temp;

        if(request==MODIFY){

            /*Setting msg*/
            msg.type=REQUEST;
            msg.content_int=request;
            msg.content_string=(String) content;
            msg.target_agent=target_agent;
            msg.sender_agent=Task_self();

            /*Getting AP address:
             * 1. Get the Agent info
             * 2. Get the AMS info*/
            temp= (Agent_info*) Task_getEnv(self_handle);
            AMS=temp->AP;

            /*Sending request*/
            Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

            /*Waiting for answer*/
            receive(timeout);

            return msg.type;
        }

        else return REFUSE;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: void Broadcast();
* Return type: Int
* Comment: request the agent to set
**********************************************************************************************/
    int Agent_Msg::broadcast(int timeout, String content){
        Agent AMS;
        Agent_info *temp;

        msg.type=REQUEST;
        msg.content_int=BROADCAST;
        msg.target_agent=NULL;
        msg.sender_agent=Task_self();
        msg.content_string=content;

        /*Getting AP address:
         * 1. Get the Agent info
         * 2. Get the AMS info*/
        temp= (Agent_info*) Task_getEnv(self_handle);
        AMS=temp->AP;

        /*Sending request*/
        Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT);

        /*Waiting for answer*/
        receive(timeout);
        return msg.type;
    }


}
