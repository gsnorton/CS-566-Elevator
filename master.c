/*******************************************************************************************************
      Denise Cole  & Greg Norton  team
      Spring 2013 CS466 w/Ken Wade, inst
      Task-master.c
      Started with code uCOSDemo2.c  Z-World, 2000

This program does the following:
A queue is defined to hold the alphabet, repeatedly. A technique is used such
that the queue is being written to and read from continuously by 2 Tasks.

Task 1: Drive motor

Task 2: Unused


********************************************************************************************************/
#class auto 			// Change default storage class for local variables: on the stack


/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-1999, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                                 V2.51
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               CONFIGURATION
*********************************************************************************************************
*/

// Redefine uC/OS-II configuration constants as necessary
#define OS_MAX_EVENTS          4       // Maximum number of events (semaphores, queues, mailboxes)
#define OS_MAX_TASKS           6  		// Maximum number of tasks system can create (less stat and idle tasks)

#define OS_TASK_CREATE_EN		 0       // Disable normal task creation
#define OS_TASK_CREATE_EXT_EN	 1       // Enable extended task creation
#define OS_TASK_STAT_EN			 1       // Enable statistics task creation
#define OS_MBOX_EN				 1			// Enable mailboxes
#define OS_MBOX_POST_EN			 1			// Enable MboxPost
#define OS_TIME_DLY_HMSM_EN	 1			// Enable OSTimeDlyHMSM
#define STACK_CNT_512	       8       // number of 512 byte stacks (application tasks + stat task + prog stack)
#define max                   40
#define true 						 1
#define false						 0
#define FORWARD	             1
#define BACKWARD	             0
#define OS_TICKS_PER_SEC     682

#use "BL4S1xx.LIB"
#use "ucos2.lib"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of bytes)       */
#define          TASK_1_ID           1
#define          TASK_2_ID           2

#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        14
/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

//      OS_EVENT        *My_Semaphore;       ?? for future use
static  int delayTics = 0;                	 /* a small test to cause a variable time context switch */
		  int channel, ack, i, j;
		  int channel_bank = 0x00;
        int seq_fwd[8] = {
   					   	0x21,0x25,0x24,0x26,0x22,0x2A,0x28,0x29    //using lower order bits
                      };
        int seq_bkwd[8] = {
   					   	0x39,0x38,0x3A,0x32,0x36,0x34,0x35,0x31    //using lower order bits
                      };

        int start		= 0x10;        						//00010000
        int fwd		= 0x20;                          //00100000
        int bkwd 		= 0x30;                          //00110000
        int halt		= 0x00;                          //00000000
 /*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void     Task1(void *data);
        void     Task2(void *data);


        void     ClearScreen();
//      void     DispStr(int x, int y, char *s);

/*
*********************************************************************************************************
*                                                    FUNCTIONS
*********************************************************************************************************
*/
int get_ack(int slave_command)
{
      int slave_ack = 0x00;
      do
      {
       channel_bank = slave_command;
       digOutBank(0,channel_bank);
      	slave_ack = digInBank(channel_bank);
      }while (slave_ack != slave_command);

      return (slave_ack);
}

//*********************************************************************************************************
void Run_Motor_Seq_Fwd(int channel_bank)
{
      	for(j=0;j<8;j++)
         {
          	channel_bank = seq_fwd[j];//seq_fwd[j];          //set each fwd trajectory value & send it
       		digOutBank(0,channel_bank);
	         OSTimeDly(1);         }
}

/*
*********************************************************************************************************
*/
void Run_Motor_Seq_Bkwd(int channel_bank)
{
         for(j=0;j<8;j++) 								//set each bkw trajectory value & send it
         {
        	channel_bank = seq_bkwd[j];//seq_bkwd[j];
       	digOutBank(0,channel_bank);
         OSTimeDly(1);
         }
}

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
    ClearScreen();                                         /* Clear the screen                         */
//	 DispStr(26, 5, "Denise Cole Lab6 CS466 Run Motors \n");
//    DispStr(26, 7, "* Press [S] or [s] to Change Direction of Motor * \n");
       // Initialize the controller

	brdInit();
		// Configure all outputs to be general digital outputs that are low
	for(channel = 0; channel < BL_DIGITAL_OUT; ++channel)
	{
 		setDigOut(channel, 0);   										// send all zeros to all the channels
   }
   	OSInit();                                              /* Initialize uC/OS-II                      */
//    My_Semaphore = OSSemCreate(1);                           /* Initialize uC/OS-II                      */

/*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/
    OSTaskCreateExt(Task1,
                   (void *)0,
                   TASK_1_PRIO,
                   TASK_1_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

 //   OSTaskCreateExt(Task2,
 //                  (void *)0,
 //                  TASK_2_PRIO,
 //                  TASK_2_ID,
 //                  TASK_STK_SIZE,
 //                  (void *)0,
 //                  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
} //end main
/*
*********************************************************************************************************
*                                               TASK #1
*
* Description:
*********************************************************************************************************
*/

void  Task1 (void *pdata)
{
    auto char  key;
    int command;
	 int direction = FORWARD;

 				printf("press F- drive fwd,\n and H to Halt motors\n\n");
      		key = getchar();
//            printf("got the kbh before the while\n");
	while(1)
   {
                                 // See if key has been pressed
//            printf("CMD selected is %c\n\n",key);


            if(key == 'F' || key == 'f') 		//forward
            {
                   direction = !direction;
							while(1)
                  	{
                  		for(i=0;i<40;i++)
			  					Run_Motor_Seq_Fwd(fwd|channel_bank);

                        for(i=0;i<40;i++)
                        Run_Motor_Seq_Bkwd(bkwd|channel_bank);
                     	if(kbhit())
	      					{
            					key = getchar();
 //                          printf("kbd read during motors run\n");
                       	 	break;
             				}
                   	 }
            }//end else if
				else if(key == 'H' || key == 'h')		//Halt
            {
							digOutBank(0, 0x00);
//                     printf("I am Halting\n");

            }//end else if
            else
            {
            	printf("unidentified command\n\n\n");
            }
//            printf("k=%c\t",key);
            if(kbhit())
	      	{
            	key = getchar();
//               printf("keybd hit in main while\n");
             }

            OSTimeDly(1);

 }//end the main while

//	 OSTimeDly(1);

} //end Task1
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description:
*********************************************************************************************************
*/

void  Task2 (void *data)          //create Master-Slave Handler
{

    for (;;)
    {

    }//end  for
  	 OSTimeDly(1);
}//end Task2
/*
*********************************************************************************************************
*                                      HELPER FUNCTIONS
*********************************************************************************************************
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