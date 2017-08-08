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
      caller=Task_self();
      clear_all_receiver();
      subscribers=0;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: private bool isRegistered(Agent aid);
* Return type: Boolean
* Comment: if returns false, sender or receiver is not registered in the same platform
**********************************************************************************************/
  bool Agent_Msg::isRegistered(Agent_AID aid){
      Agent *description_receiver=(Agent *)Task_getEnv(aid);
      Agent *description_sender=(Agent *)Task_getEnv(caller);

      if(description_receiver->agent.AP==description_sender->agent.AP) return true;
      else return false;
}

/*********************************************************************************************
* Class: Agent_Msg
* Function: private get_mailbox(Agent aid)
* Return type: Mailbox_Handle
**********************************************************************************************/
    Mailbox_Handle Agent_Msg::get_mailbox(Agent_AID aid){
        Agent * description;
        description = (Agent*) Task_getEnv(aid);
        return description->agent.mailbox_handle;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: add_receiver(Agent aid)
* Return type: Boolean. True if receiver is added successfully.
* Comment: Add receiver to list of receivers by using the agent's aid
**********************************************************************************************/
    ERROR_CODE Agent_Msg::add_receiver(Agent_AID aid_receiver){

        if(isRegistered(aid_receiver)){
            if (aid_receiver==NULL) return HANDLE_NULL;

            if(subscribers<MAX_RECEIVERS){
               receivers[subscribers]=aid_receiver;
               subscribers++;
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
    ERROR_CODE Agent_Msg::remove_receiver(Agent_AID aid){

        int i=0;
        while(i<MAX_RECEIVERS){
            if(receivers[i]==aid){
                while(i<MAX_RECEIVERS-1){
                    receivers[i]=receivers[i+1];
                    i++;
                }
                receivers[MAX_RECEIVERS-1]=NULL;
                subscribers--;
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
* Comment: Refresh the list with all the registered agents. Remove agent if it is not registered or not
* same organization
**********************************************************************************************/
    void Agent_Msg::refresh_list(){
        Agent *agent_caller,*agent_receiver;
        agent_caller = (Agent*) Task_getEnv(caller);

        for (int i=0; i<subscribers; i++){
            agent_receiver=(Agent*) Task_getEnv(receivers[i]);
            if(!isRegistered(receivers[i]) || agent_caller->agent.org!=agent_receiver->agent.org){
                remove_receiver(receivers[i]);
            }
        }
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: receive_msg(Uint32 timeout)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Receiving msg in its queue. Block call. The mailbox is obtained from the
*          task handle of the calling function of this object.
**********************************************************************************************/
    MSG_TYPE Agent_Msg::receive(Uint32 timeout){

        if (!Mailbox_pend(get_mailbox(caller), (xdc_Ptr) &msg, timeout)) msg.type=NO_RESPONSE;
        return msg.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: send(Agent aid)
* Return type: Boolean. TRUE if successful, FALSE if timeout
* Comment: Send msg to specific mailbox.
*          Set the MsgObj handle to sender's handle.
*          Send only if is registered in platform
*          Send only if same organizaton
**********************************************************************************************/
    ERROR_CODE Agent_Msg::send(Agent_AID aid_receiver,int timeout){
        msg.target_agent=aid_receiver;
        msg.sender_agent=caller;


        Agent *agent_caller,*agent_receiver;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_receiver=(Agent*) Task_getEnv(aid_receiver);

        if (!isRegistered(aid_receiver)) return NOT_REGISTERED;

        else{

            if (agent_caller->agent.org==NULL && agent_receiver->agent.org==NULL){ //Anarchy
                if(Mailbox_post(get_mailbox(aid_receiver), (xdc_Ptr)&msg, timeout)) return NO_ERROR;
                else return TIMEOUT;
            }

            else if (agent_caller->agent.org==agent_receiver->agent.org){
                if (agent_caller->agent.org->org_type==TEAM && agent_caller->agent.role==PARTICIPANT || (agent_caller->agent.org->org_type==HIERARCHY && agent_receiver->agent.role==MODERATOR)){
                    if(Mailbox_post(get_mailbox(aid_receiver), (xdc_Ptr)&msg, timeout)) return NO_ERROR;
                    else return TIMEOUT;
                }
                else return  INVALID;
            }
            else if (agent_caller->agent.affiliation==ADMIN || agent_caller->agent.affiliation==OWNER){
                if(Mailbox_post(get_mailbox(aid_receiver), (xdc_Ptr)&msg, timeout)) return NO_ERROR;
                else return TIMEOUT;
            }

            else return NOT_REGISTERED;
        }
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: send()
* Return type: Boolean. TRUE if all msgs are sent successfully to all receivers
*                       FALSE if there were an error.
* Comment: Iterate over the list. Returns last error.
**********************************************************************************************/
     ERROR_CODE Agent_Msg::send(){
        int i=0;
        ERROR_CODE error_code;
        ERROR_CODE error=NO_ERROR;

        while (receivers[i]!=NULL){
            error_code=send(receivers[i],BIOS_NO_WAIT);
            if (error_code!=NO_ERROR) error = error_code;
            i++;
        }
        return error;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_type(int type)
* Return type: NULL
* Comment: Set message type according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_type(MSG_TYPE msg_type){
        msg.type=msg_type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: set_msg_string(String content)
* Return type: NULL
* Comment: Set message body according to FIPA ACL
**********************************************************************************************/
    void Agent_Msg::set_msg_content(String msg_body){
        msg.content=msg_body;
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
    MSG_TYPE Agent_Msg::get_msg_type(){
        return msg.type;
    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: get_msg_string()
* Return type: String
* Comment: Get string content
**********************************************************************************************/
    String Agent_Msg::get_msg_content(){
        return msg.content;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_sender()
* Return type: Agent
* Comment: Get sender
**********************************************************************************************/
    Agent_AID Agent_Msg::get_sender(){
        return msg.sender_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: get_target_agent()
* Return type: Agent
* Comment: Get target aid
**********************************************************************************************/
    Agent_AID Agent_Msg::get_target_agent(){
        return msg.target_agent;
    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: Error_CODE registration(Agent target_agent)
* Return type: ERROR_CODE
* Comment: request the Agent Platform for registration service
**********************************************************************************************/
    ERROR_CODE Agent_Msg::registration(Agent_AID target_agent){
        Agent_AID AMS;
        Agent *agent_caller;
        Agent *agent_target;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_target = (Agent*) Task_getEnv(target_agent);

        if (target_agent==NULL) return HANDLE_NULL;
        else if (agent_caller->agent.org==NULL || (agent_caller->agent.org!=NULL && (agent_caller->agent.role==OWNER || agent_caller->agent.role==ADMIN))){
            if (agent_caller->agent.org==agent_target->agent.org){
                 /*Setting msg*/
                msg.type=REQUEST;
                msg.content="REGISTER";
                msg.target_agent=target_agent;
                msg.sender_agent=Task_self();

                //Get the AMS info*/
                AMS=agent_caller->agent.AP;
                /*Sending request*/
                if (Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
                else return INVALID;
            }

            else return INVALID;

        }
        else return INVALID;

    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: Error_CODE deregistration(Agent target_agent)
* Return type: ERROR_CODE
* Comment: request the Agent Platform for deregistration service
**********************************************************************************************/
    ERROR_CODE Agent_Msg::deregistration(Agent_AID target_agent){
        Agent_AID AMS;
        Agent *agent_caller;
        Agent *agent_target;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_target = (Agent*) Task_getEnv(target_agent);

        if (target_agent==NULL) return HANDLE_NULL;
        else if (agent_caller->agent.org==NULL || (agent_caller->agent.org!=NULL && (agent_caller->agent.role==OWNER || agent_caller->agent.role==ADMIN))){
            if (agent_caller->agent.org==agent_target->agent.org){
                 /*Setting msg*/
                msg.type=REQUEST;
                msg.content="DEREGISTER";
                msg.target_agent=target_agent;
                msg.sender_agent=Task_self();

                //Get the AMS info*/
                AMS=agent_caller->agent.AP;
                /*Sending request*/
                if (Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
                else return INVALID;
            }

            else return INVALID;

        }
        else return INVALID;

    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: Error_CODE suspend(Agent target_agent)
* Return type: ERROR_CODE
* Comment: request the Agent Platform for suspend service
**********************************************************************************************/
    ERROR_CODE Agent_Msg::suspend(Agent_AID target_agent){
        Agent_AID AMS;
        Agent *agent_caller;
        Agent *agent_target;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_target = (Agent*) Task_getEnv(target_agent);

        if (target_agent==NULL) return HANDLE_NULL;
        else if (agent_caller->agent.org==NULL || (agent_caller->agent.org!=NULL && (agent_caller->agent.role==OWNER || agent_caller->agent.role==ADMIN))){
            if (agent_caller->agent.org==agent_target->agent.org){
                 /*Setting msg*/
                msg.type=REQUEST;
                msg.content="SUSPEND";
                msg.target_agent=target_agent;
                msg.sender_agent=Task_self();

                //Get the AMS info*/
                AMS=agent_caller->agent.AP;
                /*Sending request*/
                if (Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
                else return INVALID;
            }

            else return INVALID;

        }
        else return INVALID;

    }
/*********************************************************************************************
* Class: Agent_Msg
* Function: Error_CODE resume(Agent target_agent)
* Return type: ERROR_CODE
* Comment: request the Agent Platform for resume service
**********************************************************************************************/
    ERROR_CODE Agent_Msg::resume(Agent_AID target_agent){
        Agent_AID AMS;
        Agent *agent_caller;
        Agent *agent_target;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_target = (Agent*) Task_getEnv(target_agent);

        if (target_agent==NULL) return HANDLE_NULL;
        else if (agent_caller->agent.org==NULL || (agent_caller->agent.org!=NULL && (agent_caller->agent.role==OWNER || agent_caller->agent.role==ADMIN))){
            if (agent_caller->agent.org==agent_target->agent.org){
                 /*Setting msg*/
                msg.type=REQUEST;
                msg.content="RESUME";
                msg.target_agent=target_agent;
                msg.sender_agent=Task_self();

                //Get the AMS info*/
                AMS=agent_caller->agent.AP;
                /*Sending request*/
                if (Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
                else return INVALID;
            }

            else return INVALID;

        }
        else return INVALID;

    }

/*********************************************************************************************
* Class: Agent_Msg
* Function: int kill(int request, Agent &target_agent)
* Return type: Int
* Comment: request the Agent Platform to kill an agent.Returns NO_ERROR
*          if request was posted correctly.
**********************************************************************************************/
    ERROR_CODE Agent_Msg::kill(Agent_AID &target_agent){
        Agent_AID AMS;
        Agent *agent_caller;
        Agent *agent_target;
        agent_caller = (Agent*) Task_getEnv(caller);
        agent_target = (Agent*) Task_getEnv(target_agent);

        if (target_agent==NULL) return HANDLE_NULL;
        else if (agent_caller->agent.org==NULL || (agent_caller->agent.org!=NULL && (agent_caller->agent.role==OWNER || agent_caller->agent.role==ADMIN))){
            if (agent_caller->agent.org==agent_target->agent.org){
                    /*Setting msg*/
                    msg.type=REQUEST;
                    msg.content="KILL";
                    msg.target_agent=target_agent;
                    msg.sender_agent=Task_self();

                    //Get the AMS info*/
                    AMS=agent_caller->agent.AP;

                    /*Sending request*/
                    if(Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)){
                        if(agent_target->agent.aid==NULL)  {
                            target_agent=NULL;
                            return NO_ERROR;
                        }
                        else return INVALID;
                    }
                    else return INVALID;
            }
            else return INVALID;
        }
        else return INVALID;
   }
/*********************************************************************************************
* Class: Agent_Msg
* Function: void restart();
* Return type: Int
* Comment: request the AMS to kill itself and create it again.
**********************************************************************************************/
    ERROR_CODE Agent_Msg::restart(){
        Agent_AID AMS;
        Agent *agent_caller;
        agent_caller = (Agent*) Task_getEnv(caller);

        msg.type=REQUEST;
        msg.content= "RESTART";
        msg.target_agent=Task_self();
        msg.sender_agent=Task_self();

        //Get the AMS info*/
        AMS=agent_caller->agent.AP;

        /*Sending request*/
        if (Mailbox_post(get_mailbox(AMS), (xdc_Ptr)&msg, BIOS_NO_WAIT)) return NO_ERROR;
        else return INVALID;

    }
}
