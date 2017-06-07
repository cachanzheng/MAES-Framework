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
*                                  Class: Agent_Platform
*
**********************************************************************************************
**********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name){
         AP_Description *ptr_AP=services.get_AP();
         ptr_AP->AMS_description.agent_name=name;
         ptr_cond=&cond;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
*          This constructor is used if user has their own conditions
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name,USER_DEF_COND*user_cond){
         AP_Description *ptr_AP=services.get_AP();
         ptr_AP->AMS_description.agent_name=name;
         ptr_cond=user_cond;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: bool init();
* Return: Boolean
* Comment: Create AMS task with default stack of 1024
***********************************************************************************************/
    bool Agent_Platform::init(){

        Agent temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;
        AP_Description *ptr_AP=services.get_AP();

        /*Assigning priority to AMS*/
        ptr_AP->AMS_description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        ptr_AP->AMS_description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=task_stack;
        taskParams.stackSize = 1024;
        taskParams.priority = Task_numPriorities-1;//Assigning max priority
        taskParams.instance->name=ptr_AP->AMS_description.agent_name;
        taskParams.env=&ptr_AP->AMS_description;
        taskParams.arg0=(UArg)&services;
        taskParams.arg1=(UArg)ptr_cond;
        ptr_AP->AMS_description.AP = Task_create(AMS_task, &taskParams, NULL);

        /*Initializing all the previously created task*/

        if (ptr_AP->AMS_description.AP!=NULL){
            temp=Task_Object_first();
            while (temp!=NULL && temp!=ptr_AP->AMS_description.AP){
                services.register_agent(temp);
                temp=Task_Object_next(temp);
            }
            return true;
        }
        else return false;
   }

/*********************************************************************************************
* Class: Agent_Platform
* Function: bool init(Task_FuncPtr action,int taskstackSize);;
* Comment: Create AMS task with user custom stack size
*          Be aware of heap size
**********************************************************************************************/
    bool Agent_Platform::init(int taskstackSize){

        Agent temp;
        Mailbox_Params mbxParams;
        Task_Params taskParams;
        AP_Description *ptr_AP=services.get_AP();

        /*Assigning priority to AMS*/
        ptr_AP->AMS_description.priority=Task_numPriorities-1;

        /*Creating mailbox*/
        Mailbox_Params_init(&mbxParams);
        ptr_AP->AMS_description.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

        /*Creating task*/
        Task_Params_init(&taskParams);
        taskParams.stack=new char[taskstackSize];
        taskParams.stackSize = taskstackSize;
        taskParams.priority = Task_numPriorities-1;//Assigning max priority
        taskParams.instance->name=ptr_AP->AMS_description.agent_name;
        taskParams.env=&ptr_AP->AMS_description;
        taskParams.arg0=(UArg)&services;
        taskParams.arg1=(UArg)ptr_cond;
        ptr_AP->AMS_description.AP= Task_create(AMS_task, &taskParams, NULL);

        /*Registering all created agents*/
        if (ptr_AP->AMS_description.AP!=NULL){
        /*Initializing all the previously created task*/
            temp=Task_Object_first();
            while (temp!=NULL && temp!=ptr_AP->AMS_description.AP){
                services.register_agent(temp);
                temp=Task_Object_next(temp);
            }

            return true;
        }
        else return false;
   }
/**********************************************************************************************
* Class: Agent_Platform
* Function:  bool search(Agent aid);
* Return: Bool
* Comment: search agent in Platform by agent aid
**********************************************************************************************/
    bool Agent_Platform::search(Agent aid){
        return services.search(aid);
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: void agent_wait (uint32 ticks)
* Return type: NULL
* Comments: When called within agent's function it will make agent sleeps defined ticks
*********************************************************************************************/
    void Agent_Platform::agent_wait(Uint32 ticks){
        Task_sleep(ticks);
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: void agent_yield()
* Return type: NULL
* Comments: It yields the processor to another readied agent of equal priority.
*           It lower priorities are on readied, the current task won't be preempted
*********************************************************************************************/
    void Agent_Platform::agent_yield(){
        Task_yield();
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: get_running_agent()
* Return type: NULL
* Comments: Returns aid of current running agent
*********************************************************************************************/
    Agent Agent_Platform::get_running_agent(){
        return Task_self();
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
    int Agent_Platform:: get_mode(Agent aid){
        return services.get_mode(aid);
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: AP_Description* get_Agent_description(Agent aid);
* Return: Agent_info
* Comment: Returns description of an agent. Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    const Agent_info *Agent_Platform::get_Agent_description(Agent aid){
        Agent_info *description;
        description=(Agent_info *)Task_getEnv(aid);
        return description;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: AP_Description* get_AP_description();
* Return: AP_Description
* Comment: Returns description of the platform Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    const AP_Description *Agent_Platform::get_AP_description(){
        return services.get_AP();
    }

/*********************************************************************************************
* Class: Agent_Platform
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns AMS task handle in case that exists
**********************************************************************************************/
    Agent Agent_Platform::get_AMS_Agent(){
        return services.get_AP()->AMS_description.AP;
    }

/*********************************************************************************************
*
*                     Unnamed namespace: For using within namespace
*
*********************************************************************************************/

    namespace{
/*********************************************************************************************
*
*                         Class: AMS_Services
*
*********************************************************************************************
*********************************************************************************************
* Class: AMS_Services
* Function: Agent_Services Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
        AMS_Services::AMS_Services(){
            int i=0;
            AP.next_available=0;
            while (i<AGENT_LIST_SIZE){
                AP.Agent_Handle[i]=(Agent) NULL;
                Task_setEnv(AP.Agent_Handle[i],NULL);
                i++;
            }
        }
/*********************************************************************************************
* Class: AMS_Services
* Function: int register_agent(Agent aid)
* Comment: Register agent to the platform only if it is unique by agent's task
**********************************************************************************************/
        int AMS_Services::register_agent(Agent aid){
            if (aid==NULL) return HANDLE_NULL;

            if (!search(aid)){
                if(AP.next_available<AGENT_LIST_SIZE){
                    Agent_info *description;
                    description=(Agent_info *)Task_getEnv(aid);
                    description->AP=AP.AMS_description.AP;
                    AP.Agent_Handle[AP.next_available]=aid;
                    AP.next_available++;
                    Task_setPri(aid, description->priority);
                    return NO_ERROR;
                }
                else return LIST_FULL;
            }
            else return DUPLICATED;
        }
/*********************************************************************************************
* Class: AMS_Services
* Function:  int deregister_agent(Agent aid);
* Comment: Deregister agent on the platform by handle. It searches inside of the list, when found,
*          the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
        int AMS_Services::deregister_agent(Agent aid){
            int i=0;
            while(i<AGENT_LIST_SIZE){
                  if(AP.Agent_Handle[i]==aid){
                          Agent_info *description;
                          suspend_agent(aid);
                          description=(Agent_info *)Task_getEnv(aid);
                          description->AP=NULL;

                          while(i<AGENT_LIST_SIZE-1){
                              AP.Agent_Handle[i]=AP.Agent_Handle[i+1];
                          i++;
                      }
                      AP.Agent_Handle[AGENT_LIST_SIZE-1]=NULL;
                      AP.next_available--;
                      break;
                  }
                  i++;
              }

              if (i==AGENT_LIST_SIZE) return NOT_FOUND;
              else return NO_ERROR;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function:  int kill_agent(Agent aid);
* Return: int
* Comment: Kill agent on the platform. It deregisters the agent first then it delete the
*           handles by agent's handle
**********************************************************************************************/
        int AMS_Services::kill_agent(Agent aid){
            int error;
            error=deregister_agent(aid);

            if(error==NO_ERROR){
                Agent_info *description;
                Mailbox_Handle m;
                description=(Agent_info *)Task_getEnv(aid);
                m=description->mailbox_handle;
                Task_delete(&aid);
                Mailbox_delete(&m);
            }

            return error;
        }

/*********************************************************************************************
* Class: AMS_Services
* Function:void suspend_agent(Agent aid)
* Return:  NULL
* Comment: suspend_agent Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
        bool AMS_Services::suspend_agent(Agent aid){
            if(search(aid)) {
                Task_setPri(aid, -1);
                return true;
            }
            else return false;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function:void restore(Agent agent)
* Return:  NULL
* Comment: Restore Agent.
**********************************************************************************************/
       bool AMS_Services::resume_agent(Agent aid){
            if(search(aid)) {
                Agent_info *description;
                description=(Agent_info *)Task_getEnv(aid);
                Task_setPri(aid,description->priority);
                return true;
            }
            else return false;
       }
/*********************************************************************************************
* Class: AMS_Services
* Function: get_AMS_AID();
* Return: Task Handle of the AMS
* Comment: returns if was successful
**********************************************************************************************/
       bool AMS_Services::modify_agent_pri(Agent aid,int pri){
            if (search(aid)){
                Agent_info *description=(Agent_info *)Task_getEnv(aid);
                Task_setPri(aid,pri);
                description->priority=pri;
                return true;
            }
            else return false;
        }
/*********************************************************************************************
* Class: AMS_Services
* Function: broadcast(MsgObj &msg)
* Return: int
* Comment: broadcast the msg to all subscribers as REQUEST type
**********************************************************************************************/
      void AMS_Services::broadcast(MsgObj *msg){
          Agent_info * description;
          int i=0;
          while(i<AP.next_available){
              if(msg->sender_agent!=AP.Agent_Handle[i]){
                  description=(Agent_info*)Task_getEnv(AP.Agent_Handle[i]);
                  Mailbox_post(description->mailbox_handle, (xdc_Ptr)msg, BIOS_NO_WAIT);
              }
              i++;
          }
      }
/*********************************************************************************************
* Class: AMS_Services
* Function: get_AP()
* Comment: Get information of Agent Platform.
**********************************************************************************************/
      AP_Description*AMS_Services::get_AP(){
          return &AP;
      }
/*********************************************************************************************
* Class: AMS_Services
* Function:  bool search();
* Return: Bool
* Comment: Search AID within list and return true if found.
*          next_available is used instead of AGENT_LIST_SIZE since it will optimize
*          search
**********************************************************************************************/
      bool AMS_Services::search(Agent aid){
          int i=0;

            while(i<AP.next_available){
                if (AP.Agent_Handle[i]==aid) break;
                i++;
            }

            if (i==AP.next_available) return false;
            else return true;
     }
/*********************************************************************************************
* Class: AMS_Services
* Function:void get_mode
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
     int AMS_Services:: get_mode(Agent aid){
         if(search(aid)){
             Task_Mode mode;
             mode=Task_getMode(aid);

             switch(mode){
             case Task_Mode_READY:
                 return ACTIVE;

             case Task_Mode_BLOCKED:
                 return WAITING;

             case Task_Mode_INACTIVE:
                 return SUSPENDED;

             case Task_Mode_TERMINATED:
                 return TERMINATED;

             default: return NULL;

             }
         }
         else return NULL;
     }
/*********************************************************************************************
* Class: -
* Function: AMS_task
* Return: NULL
* Comment: AMS task description. No visible outside of namespace. For internal use only
**********************************************************************************************/
        void AMS_task(UArg arg0, UArg arg1){
            AMS_Services *services=(AMS_Services*)arg0;
            USER_DEF_COND *cond= (USER_DEF_COND*)arg1;
            Agent_Msg msg;
            int error_msg=0;

            while(1){
                msg.receive(BIOS_WAIT_FOREVER);
                if (msg.get_msg_type()==REQUEST){
                  switch(msg.get_msg_int()){
                        case REGISTER:
                            if(cond->register_cond()){
                                error_msg=services->register_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                    msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case DEREGISTER:

                            if(cond->deregister_cond()){
                                error_msg=services->deregister_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                    msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;
                        case KILL:
                            if(cond->kill_cond()){
                                error_msg=services->kill_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case RESUME:
                            if (cond->resume_cond()){
                                error_msg=services->resume_agent(msg.get_target_agent());
                                if(error_msg==1){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case SUSPEND:
                            if (cond->suspend_cond()){
                                error_msg=services->suspend_agent(msg.get_target_agent());
                                if(error_msg==1){ //To do: extra condition
                                   msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case MODIFY:
                            if(cond->modify_cond()){
                                error_msg=services->modify_agent_pri(msg.get_target_agent(),(int)msg.get_msg_string());
                                if(error_msg==1){ //To do: extra condition
                                  msg.set_msg_type(CONFIRM);
                                }
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;

                        case BROADCAST:
                            if(cond->broadcast_cond()){
                                services->broadcast(msg.get_msg());
                                msg.set_msg_type(CONFIRM);
                            }
                            else msg.set_msg_type(REFUSE);
                            /*Respond*/
                            msg.send(msg.get_sender());
                            break;
                    }
                }//End if

                else{
                    msg.set_msg_type(NOT_UNDERSTOOD);
                    msg.send(msg.get_sender());
                }
            }//finish while
        }
    }
};
