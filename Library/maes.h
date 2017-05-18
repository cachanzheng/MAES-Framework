//maes.h
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

namespace MAES
{
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
* Class: Agent_Msg
* Comment: Predefined struct for msg object.
* Variables: int msg_type: contain type according to FIPA ACL specification
*            String body: string containing body of message
**********************************************************************************************/
    struct Agent_Msg{
        int msg_type;
        String body;
    };

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
        Agent_Build(Task_Handle t,
                    String name,
                    Mailbox_Handle m,
                    int pri,
                    Task_FuncPtr b);

        Agent_Build(Task_Handle t,
                    String name,
                    Mailbox_Handle m,
                    int pri,
                    Task_FuncPtr b,
                    int taskStackSize,
                    int msgSize,
                    int msgQueueSize);
        /*Methods*/
        void create_agent();
        String get_name();
        int get_prio();

    private:
        Task_Handle task_handle;
        Task_FuncPtr behaviour;
        Task_Params taskParams;
        Mailbox_Handle mailbox_handle;
        Mailbox_Params mbxParams;
        String agent_name;
        int task_stack_size;
        char *task_stack;
        int priority;
        int msg_size;
        int msg_queue_size;
    };


/*********************************************************************************************
* Class: Agent_AMS
* Comment:
**********************************************************************************************/
    class Agent_AMS{
        public:
            /*Methods*/
            String get_AID(Task_Handle t); /**/

            //void get_List_agent();
            //void delete_agent(Task_Handle t);
            //void suspend_agent(Task_Handle t);
            //void wait_agent(Task_Handle t); //Preemption
            //void get_agent_state(Task_Handle t);
            //void get_Arguments(Task_Handle t);
            //void get_pending_msg(Task_Handle t);
            //void get_msg_queue_size(Task_Handle t);
               //        void receive();
               //        void send();
               //        void takeDown();
               //        void setup();
               //        void set_msg_queue_size(Task_Handle);

        };


}
