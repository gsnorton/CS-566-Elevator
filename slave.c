/*****************************************************************************

   LAB7.c

******************************************************************************/

#class auto // Change default storage class for local variables: on the stack

// Redefine uC/OS-II configuration constants as necessary

#define OS_MAX_EVENTS          3       // Maximum number of events
                                       //  (semaphores, queues, mailboxes)
#define OS_MAX_TASKS           6  		// Maximum number of tasks system can
                                       //  create (less stat and idle tasks)

#define OS_TASK_CREATE_EN		 0       // Disable normal task creation
#define OS_TASK_CREATE_EXT_EN	 1       // Enable extended task creation
#define OS_TASK_STAT_EN			 1       // Enable statistics task creation
#define OS_MBOX_EN				 1			// Enable mailboxes
#define OS_MBOX_POST_EN			 1			// Enable MboxPost
#define OS_TIME_DLY_HMSM_EN	 1			// Enable OSTimeDlyHMSM
#define STACK_CNT_512	       8       // number of 512 byte stacks
                                       //  (application tasks + stat task +
                                       //    prog stack)

#define OS_TICKS_PER_SEC       682

#define OS_MUTEX_EN            1

#use "BL4S1xx.lib"
#use "ucos2.lib"

/*
*******************************************************************************
*                                   CONSTANTS
*******************************************************************************
*/

#define          TASK_STK_SIZE     512     /* Size of each task's stacks
                                              (# of bytes) */

#define          TASK_START_ID       0     /* Application tasks IDs */
#define          TASK_1_ID           1
#define          TASK_2_ID           2
#define          TASK_3_ID           3

#define          TASK_START_PRIO    10     /* Application tasks priorities */
#define          TASK_1_PRIO        11
#define          TASK_2_PRIO        12
#define          TASK_3_PRIO        13

/*
*******************************************************************************
*                                   VARIABLES
*******************************************************************************
*/

OS_EVENT        *FwdMbox;
OS_EVENT        *RevMbox;

OS_EVENT        *ChannelMutex;

/*
*******************************************************************************
*                             FUNCTION PROTOTYPES
*******************************************************************************
*/

        void     TaskStart(void *data);
static  void     TaskStartCreateTasks(void);
        void     ForwardTask(void *data);
        void     ReverseTask(void *data);
        void     CommTask(void *data);
        void     ClearScreen();
        void     DispStr(int x, int y, char *s);

/*
*******************************************************************************
*                                     MAIN
*******************************************************************************
*/

void main (void)
{
    ClearScreen();

    OSInit();

    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();
}

/*
*******************************************************************************
*                                 STARTUP TASK
*******************************************************************************
*/

void  TaskStart (void *data)
{
    static char    s[80];
    auto INT16S  key;
    auto INT8U   err;

    data = data; /* Prevent compiler warning */

    FwdMbox = OSMboxCreate((void *)0);
    RevMbox = OSMboxCreate((void *)0);

    ChannelMutex = OSMutexCreate(9, &err);

	 // Initialize the controller
	 brdInit();

    DispStr(8, 3, " OUT0\t OUT1\t OUT2\t OUT3\t OUT4\t OUT5\t OUT6\t OUT7");
	 DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

    DispStr(8, 7, "  IN0\t  IN1\t  IN2\t  IN3\t  IN4\t  IN5\t  IN6\t  IN7");
	 DispStr(8, 8, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

    DispStr(8, 11,"  FWD\t  REV\t IDLE");
    DispStr(8, 12,"-----\t-----\t-----");

	 DispStr(8, 17, "<-PRESS 'F4' to return to Edit Mode->");

    OSTaskCreateExt(ForwardTask,
                   (void *)0,
                   TASK_1_PRIO,
                   TASK_1_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(ReverseTask,
                   (void *)0,
                   TASK_2_PRIO,
                   TASK_2_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(CommTask,
                   (void *)0,
                   TASK_3_PRIO,
                   TASK_3_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    for (;;) { OSTimeDly(1); }
}

/*
*******************************************************************************
*                                     TASKS
*******************************************************************************
*/

#define USE_DISP_STR 0

void ForwardTask (void *pdata)
{
   INT8U err;

   int i;

	char *ptr;
	char display[128];

	int channel;
	int output_level;

   char channel_block;

   /* ---------------------------------------------------------------------- */

   while(1 == 1)
   {
     channel_block = 0xE1;

#if USE_DISP_STR == 1

     DispStr(10, 13, "1");

#endif

     OSMutexPend(ChannelMutex, 0, &err);

#if 0

	  for(channel = 0; channel < BL_DIGITAL_OUT; ++channel)
	  {
	     setDigOut(channel, 1);
     }

#endif

	  for(i = 0; i < 8; i++)
	  {
        digOutBank(0, channel_block);

#if USE_DISP_STR == 1

        ptr = display;

		  for(channel = 0; channel < BL_DIGITAL_OUT; channel++)
        {
			  output_level = channel_block & 0x01;
           channel_block >>= 1;
			  ptr += sprintf(ptr, "  %d\t", output_level);
		  }

		  DispStr(8, 5, display);	//update output status

#endif

        switch(i)
        {
            case 0: channel_block = 0xE5; break;
            case 1: channel_block = 0xE4; break;
            case 2: channel_block = 0xE6; break;
            case 3: channel_block = 0xE2; break;
            case 4: channel_block = 0xEA; break;
            case 5: channel_block = 0xE8; break;
            case 6: channel_block = 0xE9; break;
        }

        OSTimeDly(1);
     }

     OSMutexPost(ChannelMutex);

#if USE_DISP_STR == 1

     DispStr(10, 13, "0");

#endif

     OSMboxPend(FwdMbox, 0, &err);
  }
}

void ReverseTask (void *pdata)
{
   INT8U err;

   int i;

	char *ptr;
	char display[128];

	int channel;
	int output_level;

   char channel_block;

   /* ---------------------------------------------------------------------- */

   while(1 == 1)
   {
     channel_block = 0xF9;

#if USE_DISP_STR == 1

     DispStr(18, 13, "1");

#endif

     OSMutexPend(ChannelMutex, 0, &err);

#if 0

	  for(channel = 0; channel < BL_DIGITAL_OUT; ++channel)
	  {
        setDigOut(channel, 1);
     }

#endif

	  for(i = 7; i >= 0; i--)
	  {
        digOutBank(0, channel_block);

#if USE_DISP_STR == 1

        ptr = display;

		  for(channel = 0; channel < BL_DIGITAL_OUT; channel++)
		  {
	        output_level = channel_block & 0x01;
           channel_block >>= 1;
			  ptr += sprintf(ptr, "  %d\t", output_level);
		  }

		  DispStr(8, 5, display);	//update output status

#endif

        switch(i)
        {
            case 7: channel_block = 0xF8; break;
            case 6: channel_block = 0xFA; break;
            case 5: channel_block = 0xF2; break;
            case 4: channel_block = 0xF6; break;
            case 3: channel_block = 0xF4; break;
            case 2: channel_block = 0xF5; break;
            case 1: channel_block = 0xF1; break;
        }

        OSTimeDly(1);
     }

     OSMutexPost(ChannelMutex);

#if USE_DISP_STR == 1

     DispStr(18, 13, "0");

#endif

     OSMboxPend(RevMbox, 0, &err);
   }
}

void CommTask (void *pdata)
{
   INT8U err;

   int data, _data;

   int channel;
   int output_level;

   char *ptr;
	char display[128];

   int seen_idle = 0;

   while(1 == 1)
   {
      OSMutexPend(ChannelMutex, 0, &err);

#if 0

      for(channel = 0; channel < BL_DIGITAL_IN; ++channel)
	   {
		    setDigIn(channel);
      }

#endif

      data = digInBank(0);

#if USE_DISP_STR == 1

      ptr = display;

      _data = data;

      for(channel = 0; channel < 8; channel++)
		{
			output_level = _data & 0x01; _data >>= 1;

			ptr += sprintf(ptr, "  %d\t", output_level);
		}

		DispStr(8, 9, display);	//update output status

#endif

      switch(data & 0x07)
      {
          case 2:
          {
             if(0 == seen_idle) break;

#if USE_DISP_STR == 1

             DispStr(26, 13, "0");

#endif

             OSMboxPost(RevMbox, (void*)1);

             break;
          }

          case 3:
          {
             if(0 == seen_idle) break;

#if USE_DISP_STR == 1

             DispStr(26, 13, "0");

#endif

             OSMboxPost(FwdMbox, (void*)1);

             break;
          }

          case 0:
          {
             seen_idle = 1;

#if USE_DISP_STR == 1

             DispStr(26, 13, "1");

#endif
             break;
          }
      }

      OSMutexPost(ChannelMutex);

      OSTimeDly(1);
   }
}

/*
*******************************************************************************
*                         HELPER FUNCTIONS
*******************************************************************************
*/

nodebug void ClearScreen()
{
   printf ( "\x1Bt" );            	// Space Opens Window
}

nodebug void DispStr (int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

