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
#define NO_ERROR        0x00
#define FOUND           0x00
#define HANDLE_NULL     0x01
#define LIST_FULL       0x02
#define DUPLICATED      0x03
#define NOT_FOUND       0x04
#define TIMEOUT         0x05
/*********************************************************************************************
*   Define msg type according to FIPA ACL Message Representation in Bit-Efficient Encoding
*   Specification
**********************************************************************************************/

#define ACCEPT_PROPOSAL  0x01
#define AGREE            0x02
#define CANCEL           0x03
#define CFP              0x04
#define CONFIRM          0x05
#define DISCONFIRM       0x06
#define FAILURE          0x07
#define INFORM           0x08
#define INFORM_IF        0x09
#define INFORM_REF       0x0a
#define NOT_UNDERSTOOD   0x0b
#define PROPAGATE        0x0c
#define PROPOSE          0x0d
#define QUERY_IF         0x0f
#define QUERY_REF        0x10
#define REFUSE           0x11
#define REJECT_PROPOSAL  0x12
#define REQUEST          0x13
#define REQUEST_WHEN     0x14
#define REQUEST_WHENEVER 0x15
#define SUBSCRIBE        0x16

/*********************************************************************************************
* Class: Agent_info
* Comment: Struct containing information about the Agent;
* Variables: String AP: AP where the agent is registered;
*            String agent_name: Agent's name;
*            Mailbox_Handle m: Agent associated mailbox
*            int priority: Priority set by the user.
**********************************************************************************************/
    typedef struct Agent_info{
        String AP;
        String agent_name;
        Mailbox_Handle mailbox_handle;
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
    }AP_Description;
/*********************************************************************************************
* Class: AP_Description
* Variables: int msg_type: contain type according to FIPA ACL specification
*            String body: string containing body of message
*            Task_Handle handle: to receive information from other agents or to send info
*                                to other agents
**********************************************************************************************/
  typedef struct MsgObj{
        Task_Handle handle;
        int type;
        String body;
    }MsgObj;
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
        void print();//for debug

    private:
        Agent_info description;
        Task_Handle aid;
        Task_FuncPtr behaviour;
        char task_stack[1024];
      };


/*********************************************************************************************
* Class:   Agent_Management_Services
* Comment: API for Agent Management Services
* Variables: char task_stack[1024]: default size if additional services of AMS is required
*            int next_available: index of the Agent_Handle list where denotes the spot available
*                                in the list
*            AP_Description AP: Struct where contains the AP information.
*            Agent_info description: Description of AMS task.
**********************************************************************************************/
    class Agent_Management_Services{
    public:
        /*Constructor*/
        Agent_Management_Services(String name);

    /*Methods*/
        int register_agent(Task_Handle aid);
        void init();
        bool init(Task_FuncPtr action);
        bool init(Task_FuncPtr action,int taskstackSize);
        int kill_agent(Task_Handle aid);
        int deregister_agent(Task_Handle aid);
        bool modify_agent(Task_Handle aid,String new_AP);
        bool search(Task_Handle aid);
        bool search(String name);
        void suspend(Task_Handle aid);
        void resume(Task_Handle aid);
        void wait(Uint32 ticks);
        int get_mode(Task_Handle aid);
        void shut_down();
        AP_Description get_AP_description();
        Agent_info get_Agent_description(Task_Handle aid);
        Task_Handle get_AMS_AID();


        /*Extra services*/
        int number_of_subscribers();
        void agent_yield();
        bool set_agent_pri(Task_Handle aid,int pri);
        void broadcast(MsgObj *msg);

        void print(); //for debug

    private:
        char task_stack[1024];
        int next_available;
        AP_Description AP;
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
        int remove_receiver(Task_Handle aid_receiver);
        void clear_all_receiver();
        bool receive(Uint32 timeout);
        int send(Task_Handle aid_receiver);
        bool send();
        void set_msg_type(int type);
        void set_msg_body(String body);
        MsgObj *get_msg();
        int get_msg_type();
        String get_msg_body();
        Task_Handle get_sender();
        void print();//for debug

    private:
        Task_Handle receivers[MAX_RECEIVERS];
        int next_available;
        Task_Handle self_handle;
        Mailbox_Handle self_mailbox;
        bool isRegistered(Task_Handle aid);
        MsgObj msg;

   };



}
