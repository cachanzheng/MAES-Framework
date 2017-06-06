//maes.h
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <string.h>


namespace MAES
{
typedef Task_Handle Agent;
/*********************************************************************************************
 *                                       DEFINITIONS                                         *
*********************************************************************************************/
#define AGENT_LIST_SIZE 64
#define MAX_RECEIVERS   AGENT_LIST_SIZE-1

/*********************************************************************************************
*   Define Agent Mode
**********************************************************************************************/
#define ACTIVE          0x00
#define SUSPENDED       0x01
#define WAITING         0x02
#define TERMINATED      0x03

/*********************************************************************************************
*   Define Error handling
**********************************************************************************************/
#define NO_ERROR        0x04
#define FOUND           0x05
#define HANDLE_NULL     0x06
#define LIST_FULL       0x07
#define DUPLICATED      0x08
#define NOT_FOUND       0x09
#define TIMEOUT         0x0A
/*********************************************************************************************
*   Define msg type according to FIPA ACL Message Representation in Bit-Efficient Encoding
*   Specification
**********************************************************************************************/
#define ACCEPT_PROPOSAL  0x0B
#define AGREE            0x0C
#define CANCEL           0x0D
#define CFP              0x0E
#define CONFIRM          0x0F
#define DISCONFIRM       0x10
#define FAILURE          0x11
#define INFORM           0x12
#define INFORM_IF        0x13
#define INFORM_REF       0x14
#define NOT_UNDERSTOOD   0x15
#define PROPAGATE        0x16
#define PROPOSE          0x17
#define QUERY_IF         0x18
#define QUERY_REF        0x19
#define REFUSE           0x1A
#define REJECT_PROPOSAL  0x1B
#define REQUEST          0x1C
#define REQUEST_WHEN     0x1D
#define REQUEST_WHENEVER 0x1E
#define SUBSCRIBE        0x1F

/*********************************************************************************************
*   Define Request Action
**********************************************************************************************/
#define REGISTER        0x2A
#define DEREGISTER      0x2B
#define KILL            0x2C
#define MODIFY          0x2D
#define RESUME          0x2E
#define SUSPEND         0x2F
#define MODIFY_PRI      0x30
#define BROADCAST       0x31

/*********************************************************************************************
 *                                         TYPEDEF                                           *
**********************************************************************************************
*********************************************************************************************
* Class: Agent_info
* Comment:  Struct containing information about the Agent;
* Variables: String agent_name: Agent's name;
*            Mailbox_Handle m: Agent associated mailbox
*            Agent AP: AP handle where the agent is registered;
*            int priority: Priority set by the user.
**********************************************************************************************/
    typedef struct Agent_info{
        String agent_name;
        Mailbox_Handle mailbox_handle;
        Agent AP;
        int priority;
    }Agent_info;

/*********************************************************************************************
* Class: AP_Description
* Comment: Contains information about the AP
* Variables: String name: Name of the AP
*            Agent Agent_Handle: List of all the agents contained in the AP. Limited
*            by the number defined in AGENT_LIST_SIZE
*            Agent AMS_aid: Handle of the AMS task
*            int next_available: index of the available spot in the list
**********************************************************************************************/
    typedef struct AP_Description{
        Agent Agent_Handle[AGENT_LIST_SIZE];
        Agent_info AMS_description;
        int next_available;
    }AP_Description;
/*********************************************************************************************
* Class: AP_Description
* Variables: int msg_type: contain type according to FIPA ACL specification
*            String body: string containing body of message
*            Agent handle: to receive information from other agents or to send info
*                                to other agents
**********************************************************************************************/
    typedef struct MsgObj{
          Agent sender_agent;
          Agent target_agent;
          int type;
          String content_string;
          int content_int;
    }MsgObj;
/*********************************************************************************************
 *                           Class and functions in Unnamed namespace                        *
**********************************************************************************************/
    namespace{
        class AMS_Services{
        public:
            AMS_Services();
            /*Methods where user can override conditions*/
            int register_agent(Agent aid);
            int deregister_agent(Agent aid);
            int kill_agent(Agent aid);
            bool suspend_agent(Agent aid);
            bool resume_agent(Agent aid);
            bool modify_agent(Agent aid,Agent new_AP);
            bool set_agent_pri(Agent aid,int pri);
            void broadcast(MsgObj *msg);

            /*Methods without user conditions*/
            AP_Description *get_AP();
            bool search(Agent aid);
            int get_mode(Agent aid);

        private:
           AP_Description AP;
        };

        void AMS_task(UArg arg0,UArg arg1);  //AMS_Task
    }

/*********************************************************************************************
 *                                         CLASSES                                           *
**********************************************************************************************
**********************************************************************************************
* Class: USER_DEF_COND
* Comment: Class to be overriden for user's own conditions for AMS.
**********************************************************************************************/
    class USER_DEF_COND{
    public:
        virtual bool register_cond();
        virtual bool kill_cond();
        virtual bool deregister_cond();
        virtual bool suspend_cond();
        virtual bool resume_cond();
        virtual bool modify_cond();
        virtual bool setpri_cond();
        virtual bool broadcast_cond();
    };

/*********************************************************************************************
* Class: Agent
* Variables: Agent_info description;
*            Agent aid: task handle to agent's task/behaviour. This is used as aid
*            Task_FuncPtr: function of the agent's task/behaviour
*            char task_stack[1024]: Default char task_stack
*
* Comment: Agent construction class.
*          Add lines in cfg file to use Mailbox module:
*          var Mailbox = xdc.useModule('ti.sysbios.knl.Mailbox')
**********************************************************************************************/
    class Agent_Struct{
    public:
        /*Constructor*/
        Agent_Struct(String name);

        /*Methods*/
        Agent create(Task_FuncPtr behaviour);
        Agent create(Task_FuncPtr behaviour,int priority);
        Agent create(Task_FuncPtr behaviour,int taskstackSize,int queueSize, int priority);
        Agent create(Task_FuncPtr behaviour,UArg arg0, UArg arg1);
        Agent create(Task_FuncPtr behaviour,int taskstackSize, int queueSize, int priority,UArg arg0,UArg arg1);

    private:
        Agent_info description;
        char task_stack[1024];
      };

/*********************************************************************************************
* Class:   Agent_Platform
* Comment: API for Agent Management Services
* Variables: AMS_Services: Contains all the private and public AMS services
*            USER_DEF_COND cond: contains the default conditions for AMS private services.
*            USER_DEF_cond ptr_cond: pointer to the USER_DEF_COND where contain the user
*                                    defined functions.
*            char task_stack[1024]: default size if additional services of AMS is required
*            Agent_info description: Description of AMS task.
**********************************************************************************************/
    class Agent_Platform{
    public:
        /*Constructor*/
        Agent_Platform(String name);
        Agent_Platform(String name,USER_DEF_COND*user_cond);

    /*Methods*/
        bool init();
        bool init(int taskstackSize);

        /*Services available for all agents*/
        bool search(Agent aid);
        void agent_wait(Uint32 ticks);
        void agent_yield();
        Agent get_running_agent();
        int get_mode(Agent aid);
        const Agent_info *get_Agent_description(Agent aid);
        const AP_Description *get_AP_description();
        Agent get_AMS_Agent();

    private:
        AMS_Services services;
        USER_DEF_COND cond;
        USER_DEF_COND *ptr_cond;
        char task_stack[1024];
    };
/*********************************************************************************************
* Class: Agent_Msg
* Comment: Predefined struct for msg object.
* Variables: Agent receivers[MAX_Receivers]: list of receivers
*            int next_available: index of the receiver list where denotes the spot available
*                                in the list
*            Agent self_handle: Contains info about the calling agent.
*            Mailbox_handle self_mailbox: Contains the mailbox associated to self_handle task
*            MsgObj msg: To be used to receive and send
**********************************************************************************************/
    class Agent_Msg {
    public:
        /*Constructor*/
        Agent_Msg();

        /*Methods*/
        int add_receiver(Agent aid_receiver);
        int remove_receiver(Agent aid_receiver);
        void clear_all_receiver();
        void refresh_list();
        bool receive(Uint32 timeout);
        int send(Agent aid_receiver);
        bool send();
        void set_msg_type(int type);
        void set_msg_string(String body);
        void set_msg_int(int content);
        MsgObj *get_msg();
        int get_msg_type();
        String get_msg_string();
        int get_msg_int();
        Agent get_sender();
        Agent get_target_agent();
        int request_AP(int request, Agent target_agent,int timeout);
        int request_AP(int request, Agent target_agent,int timeout, Agent content);//Modify
        int request_AP(int request, Agent target_agent,int timeout, int content);
        int broadcast(int timeout, String content);


    private:
        MsgObj msg;
        Agent receivers[MAX_RECEIVERS];
        int next_available;
        Agent self_handle;
        Mailbox_Handle self_mailbox;
        bool isRegistered(Agent aid);
        Mailbox_Handle get_mailbox(Agent aid);

   };



}
