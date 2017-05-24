//maes.h
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <string.h>


namespace MAES
{

#define AGENT_LIST_SIZE 4
#define MAX_RECEIVERS   AGENT_LIST_SIZE-1

void behaviour(UArg arg0, UArg arg1);
/*********************************************************************************************
*   Define Error handling
**********************************************************************************************/
#define NO_ERROR        0x00
#define FOUND           0x00
#define HANDLE_NULL     0x01
#define LIST_FULL       0x02
#define DUPLICATED      0x03
#define NOT_FOUND       0x04
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
#define BROADCAST        0x17

/*********************************************************************************************
* Class: Agent_Build
* Comment: Agent construction class.
*          Add lines in cfg file to use Mailbox module:
*          var Mailbox = xdc.useModule('ti.sysbios.knl.Mailbox');
* Variables: Task_Handle task_handle: Handle to task. Used for AID
*            Task_FuncPtr behaviour: Task's behaviour
*            Task_Params taskParams: Task's parameters
*            Mailbox_Handl mailbox_handle: Handle to mailbox
*            Mailbox_Params mbxParams: Mailbox's parameters
*            String agent_name: Agent's name
*            int task_stack_size: Size to create the task stack
*            char *task_stack: Task Stack, size is defined dynamically
*            int priority: Task priority
*            int msg_size: Mailbox message size
*            int msg_queue_size: Length of mailbox
**********************************************************************************************/
    class Agent_Build{
    public:
        /*Constructor*/
        Agent_Build(String name,
                    int pri,
                    Task_FuncPtr b);
        Agent_Build(String name,
                    Task_FuncPtr b);
     //   Agent_Build(String name,
//                    int pri,
//                    Task_FuncPtr b,
//                    int taskstackSize);

        /*Methods*/
        Task_Handle create_agent();
        Task_Handle create_agent(int taskstackSize);
        void delete_agent(); //To do
        String get_name();
        int get_prio();
        Task_Handle get_AID();
        Mailbox_Handle get_mailbox();
        void agent_sleep(Uint32 ticks);

    private:
        Task_Handle task_handle;
        Task_FuncPtr behaviour;
        Task_Params taskParams;
        Mailbox_Handle mailbox_handle;
        Mailbox_Params mbxParams;
        String agent_name;
        int task_stack_size;
        char *task_stack_dyn;
        char task_stack[512];
        int priority;
        int msg_size;
        int msg_queue_size;

    };

/*********************************************************************************************
* Class: Agent_Msg
* Comment: Predefined struct for msg object.
* Variables: Mailbox_Handle mailbox_handle: Handle to the mailbox of the agent which created the
*                                           msg obj
*            Mailbox_Handle receivers: list of receivers. Set to 8.
*            Struct MsgObj made by:
*            int msg_type: contain type according to FIPA ACL specification
*            String body: string containing body of message
*            Task_Handle handle: to receive information from other agents or to send info
*                                to other agents
**********************************************************************************************/
    class Agent_Msg { //To do: Add for customized msg obj

    public:
        /*Constructor*/
        Agent_Msg(); //tO DO: Construct message with type directlt

        /*Methods*/
        int add_receiver(Task_Handle aid);
        int remove_receiver(Task_Handle aid);
        void clear_all_receiver();
        bool receive(Uint32 timeout);
        bool send(Mailbox_Handle m);
        bool send();
      //  bool broadcast();//To do
        void set_msg_type(int type);
        void set_msg_body(String body);
        int get_msg_type();
        String get_msg_body();
        String get_sender();
        void print();
    private:


        Mailbox_Handle receivers[MAX_RECEIVERS];
        int next_available;
        Task_Handle self_handle;
        struct{
            Task_Handle handle;
            int type;
            String body;

        }MsgObj;
   };

/*********************************************************************************************
* Class: AP_Description
* Comment: Contains information about the AP
* Variables: String name: Name of the AP
*            Task_Handle *ptrAgent_Handle: Pointer to the list of agents
*            AMS_aid: Task_Handle of AMS
*            AMS_mailbox: Mailbox of the AMS
**********************************************************************************************/
    struct AP_Description{

      Task_Handle *ptrAgent_Handle;
      Task_Handle AMS_aid;
      String name;
     };
/*********************************************************************************************
* Class: Agent_Management_Services
* Comment: API for Agent Management Services
* Variables: static Task_Handle Agent_Handle[AGENT_LIST_SIZE]: Contains all the running agent in th
*            the platform. Declared in static so only instance exist per platform
*            Agent_Build AMS with highest priority
**********************************************************************************************/
    class Agent_Management_Services{
        public:
            /*Methods*/
            Agent_Management_Services(String name, int taskstackSize);
            Agent_Management_Services(String name);
            bool init();
            bool init(int stackSize); //To test
            int register_agent(Task_Handle aid);
            int deregister_agent(Task_Handle aid);
            bool search(Task_Handle aid);
            bool search(String name);
            Task_Handle* return_list();
            void print();
            void modify(Task_Handle aid,Mailbox_Handle new_AP);
            AP_Description* get_description();
            Task_Handle get_AMS_AID();

            //Agent_Lifecycle
            //suspend, terminate, create, resume, invoke, execute, resource management

        private:
            int next_available;
            Task_Handle Agent_Handle[AGENT_LIST_SIZE];
            AP_Description AP;
      };

}
