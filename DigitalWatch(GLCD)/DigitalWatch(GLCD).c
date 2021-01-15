/////////////////////////////////////////////////
// HW1: ������ �ð� ����
// ������: 2015132012 �ڱٿ�
// ����: TIMER�� �̿��� ��ž ��ġ ����� �ִ� ������ �ð� ���� 
//         TIM3�� UpCounting, TIM4�� ch3���� OC���� ����  ��ž��ġ�� �ߴ� �����϶��� �ʱ�ȭ�� ����
#include "stm32f4xx.h"
#include "GLCD.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void _RCC_Init(void);
void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER3_Init(void);
void TIMER4_OC_Init(void);

void DisplayInitScreen(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

uint8_t	SW0_Flag, SW1_Flag;
uint8_t	SW6_Flag, SW7_Flag;
uint8_t	SW4_Flag, SW5_Flag;


int main(void)
{
    _GPIO_Init();  		// GPIO �ʱ�ȭ
    _EXTI_Init();		// �ܺ����ͷ�Ʈ �ʱ�ȭ
    LCD_Init();		// GLCD �ʱ�ȭ
    DelayMS(10);			

    GPIOG->ODR &= 0xFF00;	// �ʱⰪ: LED0~7 Off
    DisplayInitScreen();	// LCD �ʱ�ȭ��

    TIMER3_Init();		// ����Ÿ�̸�(TIM3) �ʱ�ȭ : up counting mode
    TIMER4_OC_Init();		// ����Ÿ�̸�(TIM3) �ʱ�ȭ : up counting mode
    while(1)
    {

    }
}

void TIMER3_Init(void)
{
	RCC->APB1ENR |= 0x02;	// RCC_APB1ENR TIMER3 Enable

	// Setting CR1 : 0x0000 
	TIM3->CR1 &= ~(1<<4);  // DIR=0(Up counter)(reset state)
	TIM3->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM3->CR1 &= ~(1<<2);	// URS=0(Update Request Source  Selection):  By one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM3->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM3->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM3->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM3->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
                            // Center-aligned mode: The counter counts UP and DOWN alternatively


    // Deciding the Period
	TIM3->PSC = 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz (0.1ms)  (1~65536)
	TIM3->ARR = 1000-1;	// Auto reload  0.1ms * 1000= 100ms

   	// Clear the Counter
	TIM3->EGR |= (1<<0);	// UG(Update generation)=1 
                        // Re-initialize the counter(CNT=0) & generates an update of registers   

	// Setting an UI(UEV) Interrupt 
	NVIC->ISER[0] |= (1<<29); // Enable Timer3 global Interrupt
 	TIM3->DIER |= (1<<0);	// Enable the Tim3 Update interrupt

	TIM3->CR1 |= (1<<0);	// Enable the Tim3 Counter (clock enable)   
}

unsigned char Sec_1_10, Sec1=7, Sec10,Sec12;

unsigned char Am_Pm=0; //AM: 0, PM: 1
void TIM3_IRQHandler(void)  	// 1ms Interrupt
{
//	static int LED_cnt=0;
    
	TIM3->SR &= ~(1<<0);	// Interrupt flag Clear
        Sec_1_10++;
        if(Sec_1_10 >= 10) //0.1*10�϶�
        {
              Sec_1_10=0;
              Sec1++;
              if(Sec1 >= 10)
              {
                  
                  Sec1=0;
                  Sec10=1;
                  Sec12=1;
                  
              }
              if(Sec12==1 && Sec10==1 && Sec1==2)
              {
                      Sec1=0;
                      Sec10=0;
                      Sec12=0;
                      if(Am_Pm == 0) //am
                      {
                        LCD_DisplayText(4,6,"AM");
                        Am_Pm = 0;
                        
                      }
                      else // pm
                      {
                        LCD_DisplayText(4,6,"PM");
                        Am_Pm = 1;
                      }
               }
              
                
        }
        LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
        LCD_DisplayChar(4,12,Sec_1_10+0x30);
        LCD_DisplayChar(4,11,':');
        LCD_DisplayChar(4,10,Sec1+0x30);
        LCD_DisplayChar(4,9,Sec10+0x30);
        
}


void TIMER4_OC_Init(void)
{
// PB8: TIM4_CH3
// PD12�� ��¼����ϰ� Alternate function(TIM4_CH3)���� ��� ����
	RCC->AHB1ENR	 |= (1<<1);	// 0x00000002, RCC_AHB1ENR GPIOB Enable : AHB1ENR.3

	GPIOB->MODER    |= (2<<16);	// 0x00020000(MODER.(25,24)=0b10), GPIOB PIN8 Output Alternate function mode 					
	GPIOB->OSPEEDR 	|= (3<<16);	// 0x03000000(OSPEEDER.(25,24)=0b11), GPIOB PIN8 Output speed (100MHz High speed)
	GPIOB->OTYPER	&= ~(1<<8);	// ~0x1000, GPIOB PIN8Output type push-pull (reset state)
	GPIOB->PUPDR    |= (1<<16); 	// 0x01000000, GPIOD PIN12 Pull-up
  					// PB8 ==> TIM4_CH3
	GPIOB->AFR[1]	|= 0x00000002;  // (AFR[1].(19~16)=0b0010): Connect TIM4 pins(PD12) to AF2(TIM3..5)
 
// Timerbase ����
	RCC->APB1ENR |= (1<<2);	// 0x04, RCC_APB1ENR TIMER4 Enable   RCC_APB1RSTR_TIM4RST

	// Setting CR1 : 0x0000 
	TIM4->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM4->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM4->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM4->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM4->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
				// Center-aligned mode: The counter counts Up and DOWN alternatively

	// Setting the Period
	TIM4->PSC = 16800-1;	// Prescaler=16800, 84MHz/16800 = 5000Hz (0.2ms)
        
	TIM4->ARR = 500-1;	// Auto reload  : 0.2ms* 500 =  100ms(period) : ���ͷ�Ʈ�ֱ⳪ ��½�ȣ�� �ֱ� ����
          
	// Update(Clear) the Counter
	TIM4->EGR |= (1<<0);    // UG: Update generation    


        // Output Compare ����
	// CCMR2(Capture/Compare Mode Register 2) : Setting the MODE of Ch3 or Ch4
	TIM4->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
	TIM4->CCMR2 &= ~(1<<2); // OC3FE=0: Output Compare 3 Fast disable 
	TIM4->CCMR2 &= ~(1<<3); // OC3PE=0: Output Compare 3 preload disable(CCR1�� �������� ���ο� ���� loading ����) 
	TIM4->CCMR2 |= (3<<4);	// OC3M=0b011 (Output Compare 3 Mode : toggle)
				// OC1REF toggles when CNT = CCR1
				
	// CCER(Capture/Compare Enable Register) : Enable "Channel 3" 
	TIM4->CCER |= (1<<8);	// CC3E=1: CC3 channel Output Enable
				// OC3(TIM4_CH3) Active: �ش���(167��)�� ���� ��ȣ��� pb8
	TIM4->CCER &= ~(1<<9);	// CC1P=0: CC3 channel Output Polarity (OCPolarity_High : OC3���� �������� ���)  

	// CC3I(CC ���ͷ�Ʈ) ���ͷ�Ʈ �߻��ð� �Ǵ� ��ȣ��ȭ(���)�ñ� ����: ��ȣ�� ����(phase) ����
	// ���ͷ�Ʈ �߻��ð�(10000 �޽�)�� 10%(1000) �ð����� compare match �߻�
	TIM4->CCR3 = 500-1;	// TIM4 CCR3 TIM4_Pulse

//	TIM4->DIER |= (1<<0);	// uie: enable tim4 update interrupt
	TIM4->DIER |= (1<<3);	// CC3IE: Enable the Tim4 CC3 interrupt

	NVIC->ISER[0] 	|= (1<<30);	// Enable Timer4 global Interrupt on NVIC

	TIM4->CR1 |= (1<<0);	// CEN: Enable the Tim4 Counter  	
}
unsigned char _Sec_1_10, _Sec1, _Sec10, stop;
void TIM4_IRQHandler(void)      //RESET: 0
{
       
        TIM4->SR &= ~(1<<3);	// CC 3 Interrupt Clear
       
        if(SW4_Flag)       //SW�� �ԷµǾ�����
        {
 //             �ð� �۵�
                 _Sec_1_10++;
                 if(_Sec_1_10 >= 10)
                {
                    
                    _Sec_1_10=0;
                    _Sec1++;
                    if(_Sec1 >= 10)
                    {
                        _Sec1=0;
                        _Sec10++;
                        if(_Sec10 >= 10)
                        {
                            _Sec10=0;
                        }
                    }
                    
                }
                
//                �ð� ȭ�� ���
                LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
                LCD_DisplayChar(5,7,_Sec_1_10+0x30);
                LCD_DisplayChar(5,6,':');
                LCD_DisplayChar(5,5,_Sec1+0x30);
                LCD_DisplayChar(5,4,_Sec10+0x30);
                
                
               
                
                
                
        }
}




void _GPIO_Init(void)
{
	// LED (GPIO G) ����
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)		0101010101010101				
	GPIOG->OTYPER	&= ~0x00FF;	    // GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) ���� 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� 
    RCC->AHB1ENR	|=  0x00000020;     // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	    // GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

void _EXTI_Init(void) //EXTI 12,13���� ����
{
    RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[3] |= 0x0077; 	// EXTI12,13�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR4) (reset value: 0x0000)	
	
	EXTI->FTSR 	|= 0x003000;	// Falling Trigger Enable  (EXTI12:PB8)

    EXTI->IMR  	|= 0x003000;  	// EXTI12,13 ���ͷ�Ʈ mask (Interrupt Enable)
		
	NVIC->ISER[1] |= ( 1 << (40-32) );   // Enable Interrupt EXTI8,9 Vector table Position ����
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 ���ͷ�Ʈ �ڵ鷯
{
	if(EXTI->PR & 0x1000) 		// EXTI12 nterrupt Pending?
	{
               BEEP(); //buzzer
		EXTI->PR |= 0x1000; 	// Pending bit Clear
               GPIOG->ODR ^= 0x10;
               GPIOG->ODR |= 0x20;
               if(GPIOG->ODR == 0x30)           //STOP WATCH ����
		SW4_Flag = 1;
               else                                     //STOP WATCH  ����
                 SW4_Flag = 0;
               
	}
	else if((EXTI->PR & 0x2000) && GPIOG->ODR == 0x20) 	// EXTI13 Interrupt Pending? +LED 5  ON only?
	{
               BEEP();  //buzzer
		EXTI->PR |= 0x2000; 	// Pending bit Clear
                GPIOG->ODR ^= 0x20;                     //LED 5 OFF
                // STOP WATCH INIT
                _Sec_1_10=0;            
                _Sec1=0;
                _Sec10=0;
		LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
                LCD_DisplayChar(5,7,_Sec_1_10+0x30);
                LCD_DisplayChar(5,6,':');
                LCD_DisplayChar(5,5,_Sec1+0x30);
                LCD_DisplayChar(5,4,_Sec10+0x30);
	}
}

void BEEP(void)			/* beep for 30 ms */
{ 	GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
		for(; Dly; Dly--);
}

void DisplayInitScreen(void)
{
    LCD_Clear(RGB_YELLOW);	// ȭ�� Ŭ����
    LCD_SetFont(&Gulim8);	// ��Ʈ : ���� 8
    
    LCD_SetPenColor(RGB_BLACK); 
    LCD_DrawRectangle(4, 6, 128, 38); // �簢�� �׸���  

    LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
    LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
    LCD_DisplayText(1,1,"Digital Watch");
    LCD_SetTextColor(RGB_GREEN);// ���ڻ� : Green
    LCD_DisplayText(2,1,"2015132012 PGW");
    LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
    LCD_DisplayText(4,1,"TIME ");
    LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
    LCD_DisplayText(4,6,"PM ");
    LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
    LCD_DisplayText(5,1,"SW");
    LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
    LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
    LCD_DisplayChar(5,7,_Sec_1_10+0x30);
    LCD_DisplayChar(5,6,':');
    LCD_DisplayChar(5,5,_Sec1+0x30);
    LCD_DisplayChar(5,4,_Sec10+0x30);
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
        		return key;
        else
        {   DelayMS(10);
            key_flag = 0;
            return key;
        }
    }
  	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
            return 0xFF00;
        else			// if new key,delay for debounce
        {	key_flag = 1;
            DelayMS(10);
            return key;
        }
	}
}


/**************************************************************************
// ���� �����ڷ�
// ������ stm32f4xx.h�� �ִ� RCC���� �ֿ� ������ 
#define HSE_STARTUP_TIMEOUT    ((uint16_t)0x05000)   // Time out for HSE start up 
typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

#define FLASH_BASE            ((uint32_t)0x08000000) // FLASH(up to 1 MB) base address in the alias region                          
#define CCMDATARAM_BASE       ((uint32_t)0x10000000) // CCM(core coupled memory) data RAM(64 KB) base address in the alias region   
#define SRAM1_BASE            ((uint32_t)0x20000000) // SRAM1(112 KB) base address in the alias region                              

#if defined(STM32F40_41xxx) 
#define SRAM2_BASE            ((uint32_t)0x2001C000) // SRAM2(16 KB) base address in the alias region                               
#define SRAM3_BASE            ((uint32_t)0x20020000) // SRAM3(64 KB) base address in the alias region                               
#endif   

#define PERIPH_BASE           ((uint32_t)0x40000000) // Peripheral base address in the alias region                                 
#define BKPSRAM_BASE          ((uint32_t)0x40024000) // Backup SRAM(4 KB) base address in the alias region                          

// Peripheral memory map  
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)

// AHB1 peripherals  
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00)
#define GPIOE_BASE            (AHB1PERIPH_BASE + 0x1000)
#define GPIOF_BASE            (AHB1PERIPH_BASE + 0x1400)
#define GPIOG_BASE            (AHB1PERIPH_BASE + 0x1800)
#define GPIOH_BASE            (AHB1PERIPH_BASE + 0x1C00)
#define GPIOI_BASE            (AHB1PERIPH_BASE + 0x2000)
#define GPIOJ_BASE            (AHB1PERIPH_BASE + 0x2400)
#define GPIOK_BASE            (AHB1PERIPH_BASE + 0x2800)
#define CRC_BASE              (AHB1PERIPH_BASE + 0x3000)
#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800)
#define FLASH_R_BASE          (AHB1PERIPH_BASE + 0x3C00)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)

// APB1 peripherals address
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400)
#define TIM12_BASE            (APB1PERIPH_BASE + 0x1800)
#define TIM13_BASE            (APB1PERIPH_BASE + 0x1C00)
#define TIM14_BASE            (APB1PERIPH_BASE + 0x2000)

// APB2 peripherals address
#define TIM1_BASE             (APB2PERIPH_BASE + 0x0000)
#define TIM8_BASE             (APB2PERIPH_BASE + 0x0400)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)
#define TIM9_BASE             (APB2PERIPH_BASE + 0x4000)
#define TIM10_BASE            (APB2PERIPH_BASE + 0x4400)
#define TIM11_BASE            (APB2PERIPH_BASE + 0x4800)

// RCC Structue
typedef struct
{
  __IO uint32_t CR;            // RCC clock control register, Address offset: 0x00  
  __IO uint32_t PLLCFGR;       // RCC PLL configuration register, Address offset: 0x04  
  __IO uint32_t CFGR;          // RCC clock configuration register, Address offset: 0x08  
  __IO uint32_t CIR;           // RCC clock interrupt register, Address offset: 0x0C  
  __IO uint32_t AHB1RSTR;      // RCC AHB1 peripheral reset register, Address offset: 0x10  
  __IO uint32_t AHB2RSTR;      // RCC AHB2 peripheral reset register, Address offset: 0x14  
  __IO uint32_t AHB3RSTR;      // RCC AHB3 peripheral reset register, Address offset: 0x18  
  __IO uint32_t APB1RSTR;      // RCC APB1 peripheral reset register, Address offset: 0x20  
  __IO uint32_t APB2RSTR;      // RCC APB2 peripheral reset register, Address offset: 0x24  
  __IO uint32_t AHB1ENR;       // RCC AHB1 peripheral clock register, Address offset: 0x30  
  __IO uint32_t AHB2ENR;       // RCC AHB2 peripheral clock register, Address offset: 0x34  
  __IO uint32_t AHB3ENR;       // RCC AHB3 peripheral clock register, Address offset: 0x38  
  __IO uint32_t APB1ENR;       // RCC APB1 peripheral clock enable register, Address offset: 0x40  
  __IO uint32_t APB2ENR;       // RCC APB2 peripheral clock enable register, Address offset: 0x44  
  __IO uint32_t AHB1LPENR;     // RCC AHB1 peripheral clock enable in low power mode register, Address offset: 0x50  
  __IO uint32_t AHB2LPENR;     // RCC AHB2 peripheral clock enable in low power mode register, Address offset: 0x54  
  __IO uint32_t AHB3LPENR;     // RCC AHB3 peripheral clock enable in low power mode register, Address offset: 0x58  
  __IO uint32_t APB1LPENR;     // RCC APB1 peripheral clock enable in low power mode register, Address offset: 0x60  
  __IO uint32_t APB2LPENR;     // RCC APB2 peripheral clock enable in low power mode register, Address offset: 0x64  
  __IO uint32_t BDCR;          // RCC Backup domain control register, Address offset: 0x70  
  __IO uint32_t CSR;           // RCC clock control & status register, Address offset: 0x74  
  __IO uint32_t SSCGR;         // RCC spread spectrum clock generation register, Address offset: 0x80  
  __IO uint32_t PLLI2SCFGR;    // RCC PLLI2S configuration register, Address offset: 0x84  
  __IO uint32_t PLLSAICFGR;    // RCC PLLSAI configuration register, Address offset: 0x88  
  __IO uint32_t DCKCFGR;       // RCC Dedicated Clocks configuration register, Address offset: 0x8C  
} RCC_TypeDef;
	
// FLASH Structue 
typedef struct
{
  __IO uint32_t ACR;      // FLASH access control register,   Address offset: 0x00  
  __IO uint32_t KEYR;     // FLASH key register,              Address offset: 0x04  
  __IO uint32_t OPTKEYR;  // FLASH option key register,       Address offset: 0x08  
  __IO uint32_t SR;       // FLASH status register,           Address offset: 0x0C  
  __IO uint32_t CR;       // FLASH control register,          Address offset: 0x10  
  __IO uint32_t OPTCR;    // FLASH option control register ,  Address offset: 0x14  
  __IO uint32_t OPTCR1;   // FLASH option control register 1, Address offset: 0x18  
} FLASH_TypeDef;

// GPIO Structue 
typedef struct
{
  __IO uint32_t MODER;    // GPIO port mode register,               Address offset: 0x00       
  __IO uint32_t OTYPER;   // GPIO port output type register,        Address offset: 0x04       
  __IO uint32_t OSPEEDR;  // GPIO port output speed register,       Address offset: 0x08       
  __IO uint32_t PUPDR;    // GPIO port pull-up/pull-down register,  Address offset: 0x0C       
  __IO uint32_t IDR;      // GPIO port input data register,         Address offset: 0x10       
  __IO uint32_t ODR;      // GPIO port output data register,        Address offset: 0x14       
  __IO uint16_t BSRRL;    // GPIO port bit set/reset low register,  Address offset: 0x18       
  __IO uint16_t BSRRH;    // GPIO port bit set/reset high register, Address offset: 0x1A       
  __IO uint32_t LCKR;     // GPIO port configuration lock register, Address offset: 0x1C       
  __IO uint32_t AFR[2];   // GPIO alternate function registers,     Address offset: 0x20-0x24  
} GPIO_TypeDef;

// EXTI Structue 
typedef struct
{
  __IO uint32_t IMR;    // EXTI Interrupt mask register, Address offset: 0x00 
  __IO uint32_t EMR;    // EXTI Event mask register, Address offset: 0x04 
  __IO uint32_t RTSR;   // EXTI Rising trigger selection register,  Address offset: 0x08
  __IO uint32_t FTSR;   // EXTI Falling trigger selection register, Address offset: 0x0C
  __IO uint32_t SWIER;  // EXTI Software interrupt event register,  Address offset: 0x10 
  __IO uint32_t PR;     // EXTI Pending register, Address offset: 0x14 
} EXTI_TypeDef;

// SYSCFG Structue 
typedef struct
{
  __IO uint32_t MEMRMP;       // SYSCFG memory remap register, Address offset: 0x00 
  __IO uint32_t PMC;          // SYSCFG peripheral mode configuration register, Address offset: 0x04
  __IO uint32_t EXTICR[4];    // SYSCFG external interrupt configuration registers, Address offset: 0x08-0x14 
  __IO uint32_t CMPCR;        // SYSCFG Compensation cell control register,Address offset: 0x20

} SYSCFG_TypeDef;

// Timer Structue 
typedef struct
{
  __IO uint16_t CR1;         // TIM control register 1, Address offset: 0x00
  __IO uint16_t CR2;         // TIM control register 2, 0x04 
  __IO uint16_t SMCR;        // TIM slave mode control register, 0x08
  __IO uint16_t DIER;        // TIM DMA/interrupt enable register, 0x0C 
  __IO uint16_t SR;          // TIM status register, 0x10 
  __IO uint16_t EGR;         // TIM event generation register, 0x14 
  __IO uint16_t CCMR1;       // TIM capture/compare mode register 1, 0x18 
  __IO uint16_t CCMR2;       // TIM capture/compare mode register 2, 0x1C 
  __IO uint16_t CCER;        // TIM capture/compare enable register, 0x20 
  __IO uint32_t CNT;         // TIM counter register, 0x24 
  __IO uint16_t PSC;         // TIM prescaler, 0x28 
  __IO uint32_t ARR;         // TIM auto-reload register, 0x2C 
  __IO uint16_t RCR;         // TIM repetition counter register, 0x30 
  __IO uint32_t CCR1;        // TIM capture/compare register 1, 0x34 
  __IO uint32_t CCR2;        // TIM capture/compare register 2, 0x38 
  __IO uint32_t CCR3;        // TIM capture/compare register 3, 0x3C 
  __IO uint32_t CCR4;        // TIM capture/compare register 4, 0x40 
  __IO uint16_t BDTR;        // TIM break and dead-time register, 0x44 
  __IO uint16_t DCR;         // TIM DMA control register, 0x48 
  __IO uint16_t DMAR;        // TIM DMA address for full transfer, 0x4C 
  __IO uint16_t OR;          // TIM option register, 0x50 
} TIM_TypeDef;


// �� �ֺ���ġ ��� ����
#define GPIOA 	((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB	((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE  	((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG   ((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH   ((GPIO_TypeDef *) GPIOH_BASE)
#define GPIOI   ((GPIO_TypeDef *) GPIOI_BASE)
#define GPIOJ   ((GPIO_TypeDef *) GPIOJ_BASE)
#define GPIOK   ((GPIO_TypeDef *) GPIOK_BASE)

#define CRC     ((CRC_TypeDef *) CRC_BASE)
#define RCC     ((RCC_TypeDef *) RCC_BASE)
#define FLASH   ((FLASH_TypeDef *) FLASH_R_BASE)

#define SYSCFG  ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define EXTI    ((EXTI_TypeDef *) EXTI_BASE)

#define TIM2    ((TIM_TypeDef *) TIM2_BASE)
#define TIM3    ((TIM_TypeDef *) TIM3_BASE)
#define TIM4    ((TIM_TypeDef *) TIM4_BASE)
#define TIM5    ((TIM_TypeDef *) TIM5_BASE)
#define TIM6    ((TIM_TypeDef *) TIM6_BASE)
#define TIM7    ((TIM_TypeDef *) TIM7_BASE)
#define TIM12   ((TIM_TypeDef *) TIM12_BASE)
#define TIM13   ((TIM_TypeDef *) TIM13_BASE)

#define TIM1    ((TIM_TypeDef *) TIM1_BASE)
#define TIM8    ((TIM_TypeDef *) TIM8_BASE)
#define TIM9    ((TIM_TypeDef *) TIM9_BASE)
#define TIM10   ((TIM_TypeDef *) TIM10_BASE)
#define TIM11   ((TIM_TypeDef *) TIM11_BASE)

#define FLASH_ACR_PRFTEN             ((uint32_t)0x00000100)
#define FLASH_ACR_ICEN               ((uint32_t)0x00000200)
#define FLASH_ACR_DCEN               ((uint32_t)0x00000400)
#define FLASH_ACR_ICRST              ((uint32_t)0x00000800)
#define FLASH_ACR_DCRST              ((uint32_t)0x00001000)
#define FLASH_ACR_BYTE0_ADDRESS      ((uint32_t)0x40023C00)
#define FLASH_ACR_BYTE2_ADDRESS      ((uint32_t)0x40023C03)

#define FLASH_ACR_LATENCY_5WS        ((uint32_t)0x00000005)

typedef struct {
  __IO uint32_t ISER[8];  // Offset: 0x000 Interrupt Set Enable Register    
  __IO uint32_t ICER[8];  // Offset: 0x080 Interrupt Clear Enable Register  
  __IO uint32_t ISPR[8];  // Offset: 0x100 Interrupt Set Pending Register   
  __IO uint32_t ICPR[8];  // Offset: 0x180 Interrupt Clear Pending Register
  __IO uint32_t IABR[8];  // Offset: 0x200 Interrupt Active bit Register      
  __IO uint8_t  IP[240];  // Offset: 0x300 Interrupt Priority Register (8Bit) 
  __O  uint32_t STIR;  // Offset: 0xE00 Software Trigger Interrupt Register    
}  NVIC_Type; 
     
// Memory mapping of Cortex-M4 Hardware 
#define SCS_BASE     (0xE000E000)    // System Control Space Base Address 
#define NVIC_BASE   (SCS_BASE +  0x0100)  // NVIC Base Address  
#define NVIC        ((NVIC_Type *)  NVIC_BASE) // NVIC configuration struct                                           

*/ 