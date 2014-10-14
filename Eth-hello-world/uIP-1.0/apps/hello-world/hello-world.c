/**
 * \addtogroup helloworld
 * @{
 */

/**
 * \file
 *         An example of how to write uIP applications
 *         with protosockets.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/*
 * This is a short example of how to write uIP applications using
 * protosockets.
 */

/*
 * We define the application state (struct hello_world_state) in the
 * hello-world.h file, so we need to include it here. We also include
 * uip.h (since this cannot be included in hello-world.h) and
 * <string.h>, since we use the memcpy() function in the code.
 */
#include "hello-world.h"
#include "uip.h"
#include <string.h>
#include "stm32f10x.h"
extern void USART_Print(USART_TypeDef* USARTx, unsigned char* TxBuf);
extern void USART_SendData(USART_TypeDef* USARTx, uint16_t Data);
extern void USART_PrintString(uint32_t DATA );
static uint8_t flag = 0x01;

/*
 * Declaration of the protosocket function that handles the connection
 * (defined at the end of the code).
 */
static int handle_connection(struct hello_world_state *s);
/*---------------------------------------------------------------------------*/
/*
 * The initialization function. We must explicitly call this function
 * from the system initialization code, some time after uip_init() is
 * called.
 */
void
hello_world_init(void)
{
    /* We start to listen for connections on TCP port 1000. */
    uip_listen(HTONS(1000));
}
/*---------------------------------------------------------------------------*/
/*
 * In hello-world.h we have defined the UIP_APPCALL macro to
 * hello_world_appcall so that this funcion is uIP's application
 * function. This function is called whenever an uIP event occurs
 * (e.g. when a new connection is established, new data arrives, sent
 * data is acknowledged, data needs to be retransmitted, etc.).
 */
void
hello_world_appcall(void)
{
    int i = 0;
    /*
     * The uip_conn structure has a field called "appstate" that holds
     * the application state of the connection. We make a pointer to
     * this to access it easier.
     */
    struct hello_world_state *s = &(uip_conn->appstate);
    //USART_Print(USART1, "xxxx");
    /*
     * If a new connection was just established, we should initialize
     * the protosocket in our applications' state structure.
     */
    if(uip_connected()) {//新的客户端连接
        PSOCK_INIT(&s->p, s->inputbuffer, sizeof(s->inputbuffer));
        //USART_Print(USART1, "x");
    } else if(uip_closed() || uip_aborted() || uip_timedout()) {//客户端异常
        //USART_Print(USART1, "y");
    } else if(s != NULL) {
        if(uip_newdata()) { //服务器端收到客户端数据
            if(flag & 0x01)
            {
                GPIO_SetBits(GPIOC, GPIO_Pin_3);
                flag = 0x00;
            }
            else
            {
                flag = 0x01;
                GPIO_ResetBits(GPIOC, GPIO_Pin_3);
            }
            //USART_Print(USART1, "z");
            USART_SendData(USART1 , uip_datalen());
            while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
            for(i = 0; i < uip_len; ++i) {
                USART_SendData(USART1 , *((char*)uip_appdata + i));
                while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
            }

        }
        else if(uip_acked()) { //服务器发送数据后收到ACK
            //USART_Print(USART1, "a");
        }
        else {
            //USART_Print(USART1, "b");
        }
    }
    else {
        //USART_Print(USART1, "c");
        //  uip_abort();
    }
    /*
     * Finally, we run the protosocket function that actually handles
     * the communication. We pass it a pointer to the application state
     * of the current connection.
     */
    handle_connection(s);
}
/*---------------------------------------------------------------------------*/
/*
 * This is the protosocket function that handles the communication. A
 * protosocket function must always return an int, but must never
 * explicitly return - all return statements are hidden in the PSOCK
 * macros.
 */
static int
handle_connection(struct hello_world_state *s)
{
    PSOCK_BEGIN(&s->p);

    PSOCK_SEND_STR(&s->p, "Hello. What is your name?\n");
    PSOCK_READTO(&s->p, '\n');
    strncpy(s->name, s->inputbuffer, sizeof(s->name));
    PSOCK_SEND_STR(&s->p, "Hello ");
    PSOCK_SEND_STR(&s->p, s->name);
    PSOCK_CLOSE(&s->p);

    PSOCK_END(&s->p);
}
/*---------------------------------------------------------------------------*/
