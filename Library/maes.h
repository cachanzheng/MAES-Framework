//maes.h
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <string.h>


namespace MAES
{

#define AGENT_LIST_SIZE 32
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
* Class: Agent_info
* Comment: Struct containing information about the Agent;
* Variables: String agent_name: Agent's name;
*            Mailbox_Handle m: Agent associated mailbox
*            Task_Handle AP: AP handle where the agent is registered;
*            int priority: Priority set by the user.
**********************************************************************************************/
    typedef struct Agent_info{
        String agent_name;
        Mailbox_Handle mailbox_handle;
        Task_Handle AP;
        int priority;
    }Agent_info;

/*********************************************************************************************
* Class: AP_Description
* Comment: Contains information about the AP
* Variables: String name: Name of the AP
*            Task_Handle Agent_Handle: List of all the agents contained in the AP. Limited
*            by the number defined in AGENT_LIST_SIZE
*            Task_handle AMS_aid: Handle of the AMS task
**********************************************************************************************/
    typedef struct AP_Description{
        String name;
        Task_Handle Agent_Handle[AGENT_LIST_SIZE];
        Task_Handle AMS_aid;
        int next_available;
    }AP_Description;
/*********************************************************************************************
* Class: AP_Description
* Variables: int msg_type: contain type according to FIPA ACL specification
*            String body: string containing body of message
*            Task_Handle handle: to receive information from other agents or to send info
*                                to other agents
**********************************************************************************************/
    typedef struct MsgObj{
          Task_Handle sender_agent;
          Task_Handle target_agent;
          int type;
          String content_string;
          int content_int;
    }MsgObj;
/*********************************************************************************************
*  Unnamed namespace for using within namespace
**********************************************************************************************/
  namespace{
        class AMS_Services{
        public:
            AMS_Services();
            AP_Description *get_AP();
            bool search(Task_Handle aid);
            int register_agent(Task_Handle aid);
            int kill_agent(Task_Handle aid);
            int deregister_agent(Task_Handle aid);
            bool suspend_agent(Task_Handle aid);
            bool modify_agent(Task_Handle aid,Task_Handle new_AP);
            bool resume_agent(Task_Handle aid);
            bool set_agent_pri(Task_Handle aid,int pri);
            int get_mode(Task_Handle aid);
            void broadcast(MsgObj *msg);

        private:
            AP_Description AP;

        };
        void AMS_task(UArg arg0,UArg arg1);
    }
/*********************************************************************************************
* Class: Agent
* Variables: Agent_info description;
*            Task_handle aid: task handle to agent's task/behaviour. This is used as aid
*            Task_FuncPtr: function of the agent's task/behaviour
*             char task_stack[1024]: Default char task_stack
*
* Comment: Agent construction class.
*          Add lines in cfg file to use Mailbox module:
*          var Mailbox = xdc.useModule('ti.sysbios.knl.Mailbox')
**********************************************************************************************/
    class Agent{
    public:
        /*Constructor*/
        Agent(String name,
              Task_FuncPtr b);

        /*Methods*/
        bool init_agent();
        bool init_agent(int taskstackSize,int queueSize, int priority);
        bool init_agent(UArg arg0, UArg arg1);
        bool init_agent(int taskstackSize, int queueSize, int priority,UArg arg0,UArg arg1);
        bool destroy_agent();
        Task_Handle get_AID();
        bool isRegistered();

    private:
        Agent_info description;
        Task_Handle aid;
        Task_FuncPtr behaviour;
        char task_stack[1024];
      };


/*********************************************************************************************
* Class:   Agent_Platform
* Comment: API for Agent Management Services
* Variables: char task_stack[1024]: default size if additional services of AMS is required
*            int next_available: index of the Agent_Handle list where denotes the spot available
*                                in the list
*            AP_Description AP: Struct where contains the AP information.
*            Agent_info description: Description of AMS task.
**********************************************************************************************/
    class Agent_Platform{
    public:
        /*Constructor*/
        Agent_Platform(String name);

    /*Methods*/
        bool init();
        bool init(Task_FuncPtr action,int taskstackSize);

        /*Services available for all agents*/
        bool search(Task_Handle aid);
        bool search(Agent *a);
        void agent_wait(Uint32 ticks);
        void agent_yield();
        Task_Handle get_running_agent_aid();
        int get_mode(Task_Handle aid);
        const Agent_info *get_Agent_description(Task_Handle aid);
        const Agent_info *get_Agent_description(Agent *a);
        const AP_Description *get_AP_description();
        Task_Handle get_AMS_AID();
        int number_of_subscribers();

    private:
        AMS_Services services;
        char task_stack[1024];
        Agent_info description;

    };

/*********************************************************************************************
* Class: Agent_Msg
* Comment: Predefined struct for msg object.
* Variables: Task_Handle receivers: list of receivers
*            int next_available: index of the receiver list where denotes the spot available
*                                in the list
*            Task_Handle self_handle: Contains info about the calling agent.
*            Mailbox_handle self_mailbox: Contains the mailbox associated to self_handle task
*            MsgObj msg: To be used to receive and send
**********************************************************************************************/
    class Agent_Msg {
    public:
        /*Constructor*/
        Agent_Msg();

        /*Methods*/
        int add_receiver(Task_Handle aid_receiver);
        int add_receiver(Agent *a);
        int remove_receiver(Task_Handle aid_receiver);
        int remove_receiver(Agent *a);
        void clear_all_receiver();
        void refresh_list();
        bool receive(Uint32 timeout);
        int send(Task_Handle aid_receiver);
        int send(Agent *a);
        bool send();
        void set_msg_type(int type);
        void set_msg_string(String body);
        void set_msg_int(int content);
        MsgObj *get_msg();
        int get_msg_type();
        String get_msg_string();
        int get_msg_int();
        Task_Handle get_sender();
        Task_Handle get_target_agent();
        int request_AP(int request, Task_Handle target_agent,int timeout);
        int request_AP(int request, Agent *a,int timeout);
        int request_AP(int request, Task_Handle target_agent,int timeout, Task_Handle content);//Modify
        int request_AP(int request, Task_Handle target_agent,int timeout, int content);
        int request_AP(int request, Agent *a,int timeout, int content);
        int broadcast(int timeout);


    private:
        Task_Handle receivers[MAX_RECEIVERS];
        int next_available;
        Task_Handle self_handle;
        Mailbox_Handle self_mailbox;
        bool isRegistered(Task_Handle aid);
        MsgObj msg;

   };



}
