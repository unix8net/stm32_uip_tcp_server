/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/**********************������UIP����ģ�鶨��***********************************/
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "timer.h"

#include "hello-world.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */
/**********************UIP����ģ�鶨��END***********************************/
extern void USART1_Config(void);
extern void USART_Print(USART_TypeDef* USARTx, unsigned char* TxBuf);
extern void USART_SendData(USART_TypeDef* USARTx, uint16_t Data);
extern void USART_PrintString(uint32_t DATA );

extern void etherdev_init(void);
extern unsigned int etherdev_read(void);
extern void etherdev_send(void);
extern void SPI2_Init(void);

void  Delay (u32 nCount)
{
    for(; nCount != 0; nCount--);
}


void GPIO_Config()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO, ENABLE);

//====================LED����PC2��PC3==============================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

}

/*******************************************
 * ��ʼ��NVIC�жϿ���������
 * ע��:STM32��Ȼ���60��������жϣ���ʵ����ÿ���ж�ֻ����4bit
 * ������4bit���жϽ����˷ֵȼ������飩����Ϊ��ռ���ȼ��������ȼ���
 * оǶstm32  @2013-6-2ע��
*********************************************/
void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	    	//����2λ�����ȼ�����ռ���ȼ�����2λ�����ȼ�����Ӧ���ȼ���
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  			//�ⲿ�ж���0,1�ֱ��ӦPC0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 	//io�����ж���ռ���ȵȼ�Ϊ�ڶ�����
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 			//io�����ж������ȵȼ�Ϊ�ڶ�����
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}


void Timer_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    TIM_DeInit(TIM2);

    TIM_TimeBaseStructure.TIM_Period=1000;		 					//�Զ���װ�ؼĴ������ڵ�ֵ(����ֵ)
    //���壺��ʱ���ж�Ҫ�����²��ܽ��жϣ�����2000��ʾҪ��2000�½��ж�
    //�ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж�
    TIM_TimeBaseStructure.TIM_Prescaler= (720 - 1);				    //ʱ��Ԥ��Ƶ��   ���磺ʱ��Ƶ��=72MHZ/(ʱ��Ԥ��Ƶ+1)
    //ʱ��Ƶ�ʣ�һ���ӿ��������¡�������72MHZ/720=100K����1s����100K��
    //���ʱ��Timer��һ���ӻ���100K�Σ�����ÿ��2000�¾ͽ��жϣ���ôÿ�ζ�ʱʱ��20ms
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 			//������Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 		//���ϼ���ģʽ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);							//�������жϱ�־
    //TIM_PrescalerConfig(TIM2,0x8C9F,TIM_PSCReloadMode_Immediate);	//ʱ�ӷ�Ƶϵ��36000�����Զ�ʱ��ʱ��Ϊ2K
    //TIM_ARRPreloadConfig(TIM2, DISABLE);							//��ֹARRԤװ�ػ�����
    TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_Trigger,ENABLE);
    TIM_Cmd(TIM2, ENABLE);											//����ʱ��

}

int main(void)
{
    int i;
    uip_ipaddr_t ipaddr;
    struct timer periodic_timer, arp_timer;

    SystemInit();
    GPIO_Config();		//LED�����������
    USART1_Config();	//���ڴ�ӡ������Ϣ
    NVIC_Config();
    SPI2_Init();
    Timer_Config();	   //����uIP��ʱ�ӣ�ÿ10ms��һ��

    /*���� TCP��ʱ����ʱ��� ARP�ϻ�ʱ��*/
    timer_set(&periodic_timer, CLOCK_SECOND / 2);
    timer_set(&arp_timer, CLOCK_SECOND * 10);
    etherdev_init();

    uip_init();
    uip_arp_init();

    uip_ipaddr(ipaddr, 192,168,0,169);
    uip_sethostaddr(ipaddr);
    uip_ipaddr(ipaddr, 192,168,0,1);
    uip_setdraddr(ipaddr);
    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
		hello_world_init();
		timer_restart(&periodic_timer);
		timer_restart(&arp_timer);
    while(1) {
        uip_len = etherdev_read(); 				  /*��ѯ������������*/
        if(uip_len>0) {   							/*�������������Э�鴦��*/
            if(BUF->type == htons(UIP_ETHTYPE_IP)) {	   /*�յ�����IP���ݣ����� uip_input()����*/
                uip_arp_ipin();
                uip_input();
                if(uip_len>0) {					      /*����������uip_buf�������ݣ������ etherdev_send ���ͳ�ȥ*/
                    uip_arp_out();
                    etherdev_send();
                }
            } else if (BUF->type==htons(UIP_ETHTYPE_ARP)) { /*�յ����� ARP���ݣ����� uip_arp_arpin()����*/
                uip_arp_arpin();
							//���������������ARP�������ǻ���Ҫ��Ӧ����
                if(uip_len>0) {
                    etherdev_send();
                }
            }
        } else if(timer_expired(&periodic_timer)) {	//0.5�붨ʱ����ʱ
					//USART_Print(USART1, "periodic");
            timer_reset(&periodic_timer);			//��λ0.5�붨ʱ��
            for(i=0; i<UIP_CONNS; i++) {
                uip_periodic(i);
                if(uip_len>0) {
									//�ڷ���IIP��֮ǰ�������uip_arp_out
                    uip_arp_out();
                    etherdev_send();
                }
            }
#if UIP_UDP
            for(i=0; i<UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);
                if(uip_len > 0) {
                    uip_arp_out();
                    etherdev_send();
                }
            }
#endif
            if(timer_expired(&arp_timer)) {
                timer_reset(&arp_timer);
                uip_arp_timer();
            }
        }
    }

}


