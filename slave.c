/*****************************************************************************

   LAB7.c

******************************************************************************/

#class auto // Change default storage class for local variables: on the stack

// Redefine uC/OS-II configuration constants as necessary

#define OS_MAX_EVENTS          4       // Maximum number of events
                                       //  (semaphores, queues, mailboxes)
#define OS_MAX_TASKS           6       // Maximum number of tasks system can
                                       //  create (less stat and idle tasks)

#define OS_TASK_CREATE_EN      0       // Disable normal task creation
#define OS_TASK_CREATE_EXT_EN  1       // Enable extended task creation
#define OS_TASK_STAT_EN        1       // Enable statistics task creation
#define OS_MBOX_EN             1       // Enable mailboxes
#define OS_MBOX_POST_EN        1       // Enable MboxPost
#define OS_TIME_DLY_HMSM_EN    1       // Enable OSTimeDlyHMSM
#define STACK_CNT_512          8       // number of 512 byte stacks
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
#define          TASK_4_ID           4

#define          TASK_START_PRIO    10     /* Application tasks priorities */
#define          TASK_1_PRIO        11
#define          TASK_2_PRIO        12
#define          TASK_3_PRIO        13
#define          TASK_4_PRIO        40

/*
*******************************************************************************
*                                   VARIABLES
*******************************************************************************
*/

OS_EVENT        *FwdMbox;
OS_EVENT        *RevMbox;
OS_EVENT        *DoneMbox;

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
        void     MD5HashTask(void *data);
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

    FwdMbox =  OSMboxCreate((void *)0);
    RevMbox =  OSMboxCreate((void *)0);
    DoneMbox = OSMboxCreate((void *)0);

    ChannelMutex = OSMutexCreate(9, &err);

    // Initialize the controller
    brdInit();

    DispStr(8, 3, " OUT0\t OUT1\t OUT2\t OUT3\t OUT4\t OUT5\t OUT6\t OUT7");
    DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

    DispStr(8, 7, "  IN0\t  IN1\t  IN2\t  IN3\t  IN4\t  IN5\t  IN6\t  IN7");
    DispStr(8, 8, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

    DispStr(8, 11,"  FWD\t  REV\t IDLE");
    DispStr(8, 12,"-----\t-----\t-----");

    DispStr(8, 20, "<-PRESS 'F4' to return to Edit Mode->");

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

#if 1

    OSTaskCreateExt(MD5HashTask,
                   (void *)0,
                   TASK_4_PRIO,
                   TASK_4_ID,
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#endif

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

   register int i;

   char *ptr;
   char display[128];

   int channel;
   int output_level;

   char channel_block;

   /* ---------------------------------------------------------------------- */

   while(1 == 1)
   {
     channel_block = 0xE1;

     OSMutexPend(ChannelMutex, 0, &err);

#if USE_DISP_STR == 1

     DispStr(10, 13, "1");

#endif

     for(i = 0; i < 8; i++)
     {
        if((digInBank(0) ^ 0xF8) == 0)
        {

            break;
        }

        digOutBank(0, channel_block);
        OSTimeDly(1);

#if USE_DISP_STR == 1

        ptr = display;

        for(channel = 0; channel < BL_DIGITAL_OUT; channel++)
        {
           output_level = channel_block & 0x01;
           channel_block >>= 1;
           ptr += sprintf(ptr, "  %d\t", output_level);
        }

        DispStr(8, 5, display);  //update output status

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

     }

#if USE_DISP_STR == 1

     DispStr(10, 13, "0");

#endif

     OSMutexPost(ChannelMutex);

     OSMboxPend(FwdMbox, 0, &err);
  }
}

void ReverseTask (void *pdata)
{
   INT8U err;

   register int i, data;

   char *ptr;
   char display[128];

   int channel;
   int output_level;

   char channel_block;

   /* ---------------------------------------------------------------------- */

   while(1 == 1)
   {
     channel_block = 0xF9;

     OSMutexPend(ChannelMutex, 0, &err);

#if USE_DISP_STR == 1

     DispStr(18, 13, "1");

#endif

     for(i = 7; i >= 0; i--)
     {
        if((digInBank(0) ^ 0xF8) == 0)
        {

            break;
        }

        digOutBank(0, channel_block);
        OSTimeDly(1);

#if USE_DISP_STR == 1

        ptr = display;

        for(channel = 0; channel < BL_DIGITAL_OUT; channel++)
        {
           output_level = channel_block & 0x01;
           channel_block >>= 1;
           ptr += sprintf(ptr, "  %d\t", output_level);
        }

        DispStr(8, 5, display);  //update output status

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
     }

#if USE_DISP_STR == 1

     DispStr(18, 13, "0");

#endif

     OSMutexPost(ChannelMutex);

     OSMboxPend(RevMbox, 0, &err);
   }
}

void CommTask (void *pdata)
{
   INT8U err;

   register int data;

   int channel;
   int output_level;

   char *ptr;
   char display[128];

   int seen_idle = 0;

   while(1 == 1)
   {
      OSMutexPend(ChannelMutex, 0, &err);

      data = digInBank(0);

      switch(data ^ 0xF8)
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

             digOutBank(0, 0);

             break;
          }
      }

#if USE_DISP_STR == 1

      ptr = display;

      for(channel = 0; channel < 8; channel++)
      {
         output_level = data & 0x01; data >>= 1;

         ptr += sprintf(ptr, "  %d\t", output_level);
      }

      DispStr(8, 9, display); //update output status

#endif

      OSMutexPost(ChannelMutex);

      OSTimeDly(1);
   }
}

/*
*******************************************************************************
*                        CPU-INTENSIVE TASK
*
*                MD5 code originated from RSA Security Inc.
*******************************************************************************
*/

typedef struct _MD5_CTX
{
    unsigned long state[4];
    unsigned long count[2];

    unsigned char buffer[64];
}
MD5_CTX;


void MD5Init(MD5_CTX *context)
{
    context->count[0] = context->count[1] = 0;

    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

static void encode(unsigned char *output, unsigned long *input,
                   unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
  {
    output[j]   = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

static void decode(unsigned long *output, unsigned char *input,
                   unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = input[j] | (input[j+1] << 8) |
                (input[j+2] << 16) | (input[j+3] << 24);
}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define _F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define _G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define _H(x, y, z) ((x) ^ (y) ^ (z))
#define _I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.*/
#define FF(a, b, c, d, x, s, ac) { \
 (a) += _F ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += _G ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += _H ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += _I ((b), (c), (d)) + (x) + (unsigned long)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

void MD5Transform(unsigned long state[4], unsigned char block[64])
{
  unsigned long a = state[0], b = state[1],
                c = state[2], d = state[3], x[16];

  decode(x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

   /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.*/
  memset(x, 0, sizeof (x));
}

void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen)
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((context->count[0] += ((unsigned long)inputLen << 3))
        < ((unsigned long)inputLen << 3)) context->count[1]++;
    context->count[1] += ((unsigned long)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible. */
    if (inputLen >= partLen)
    {
        memcpy(&context->buffer[index], input, partLen);
        MD5Transform (context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform (context->state, &input[i]);

        index = 0;
    }
    else
        i = 0;

    memcpy(&context->buffer[index], &input[i], inputLen-i);
}

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void MD5Final(unsigned char digest[16], MD5_CTX *context)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  encode(bits, context->count, 8);

  /* Pad out to 56 mod 64.*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);

  /* Store state in digest */
  encode (digest, context->state, 16);

  /* Zeroize sensitive information.*/
  memset (context, 0, sizeof (*context));
}

void MD5HashTask (void *pdata)
{
   INT8U err;

   int i;
   char *ptr;
   char display[128];

   MD5_CTX context;
   unsigned char digest[16];

   while(1 == 1)
   {
      MD5Init(&context);
      MD5Update(&context, "", 0);
      MD5Final(digest, &context);

#if USE_DISP_STR == 1

      ptr = display;

      for(i = 0; i < 16; i++)
          ptr += sprintf(ptr, "%02X", digest[i]);

      OSMutexPend(ChannelMutex, 0, &err);
      DispStr(8, 16, display);
      OSMutexPost(ChannelMutex);

#endif

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
   printf ( "\x1Bt" );              // Space Opens Window
}

nodebug void DispStr (int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

