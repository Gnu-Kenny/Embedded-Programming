#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define NAVI_UP		0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_RIGHT	0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);	// Joystick �Է� �Լ�
void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);


unsigned int CoinInput,Total; //����,������ ��Ÿ���� ����
unsigned char LowByte,HighByte; //FRAM�� ��,���� �ڸ�, ���� �ڸ��� ���� ���� ���� ����
uint8_t	SW0_Flag, SW1_Flag,SW3_Flag,SW4_Flag;  //SW0,1,3,4�� ���¸� ǥ���� ����
uint8_t        StickUP_Flag,StickL_Flag,StickR_Flag;            //���̽�ƽ�� ���¸� ǥ���� ����
int cp=9,sg=5,mk=5,cf=9,NoC;                                    //��,����,����,���θ� ��Ÿ�� ������ �Ǹ��� Ŀ�� ���� ��Ÿ���� ����
int RF_Flag;            //Refill �� �ʿ��� ���������� ǥ���� ����
uint8_t SW2_Push =0; //���ͷ�Ʈ ��ø�� ���� ����

int main(void)
{
	_GPIO_Init(); 	// GPIO (LED,SW,Buzzer,Joy stick) �ʱ�ȭ
        _EXTI_Init();	// EXTI �ʱ�ȭ
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);

	Fram_Init();                    // FRAM �ʱ�ȭ H/W �ʱ�ȭ
	Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
        LowByte = Fram_Read(625);       //�ʱ�ȭ ������ total ���� ���� �ڸ�,���� �ڸ��� ���� ����
        HighByte = Fram_Read(626);      //�ʱ�ȭ ������ total ���� ���� �ڸ��� ���� ����
        Total = (unsigned int)HighByte;         //���� �ڸ� ����
        Total = (unsigned int)((Total<<8) + LowByte);//���� �ڸ��� 8��Ʈ ����Ʈ�ϰ� ��,���� �ڸ��� ����
        
	DisplayInitScreen();    // LCD �ʱ�ȭ��
//	BEEP();

	GPIOG->ODR &= ~0x00FF;	// LED �ʱⰪ: LED0~7 Off
//	LCD_DisplayChar(1,10,Fram_Read(50)+0x30); //FRAM 50���� ����� data(1byte) �о� LCD�� ǥ��
                                                                   
	while(1)
	{
        
                switch(JOY_Scan())	// �Էµ� Joystick ���� �з�
		{
                        case NAVI_UP : 	// Joystick UP
                          if (StickUP_Flag==0 && RF_Flag==0) { //���̽�ƽ�� �� �����̰� ������ �ʿ����� ������ ����
                                        BEEP();
                                        LCD_SetBackColor(RGB_BLUE);     //S���� ���̽�ƽ �Ķ��� �ڽ� �Ͼ� ���ڷ� �ʱ�ȭ
                                        LCD_SetTextColor(RGB_WHITE);
                                        LCD_DisplayChar(3,1,'B');
                                        LCD_DisplayChar(3,3,'M');
                                        LCD_SetBackColor(RGB_BLUE);
                                        LCD_SetTextColor(RGB_RED);      //S����
                                        LCD_DisplayChar(2,2,'S');
                                        StickR_Flag=0;
					StickL_Flag = 0;
					StickUP_Flag = 1;               //UP�� ����
				}

                                break;
        		
                        case NAVI_LEFT:	// Joystick LEFT	
				if (StickL_Flag==0 && RF_Flag==0) { //���̽�ƽ�� LEFT �����̰� ������ �ʿ����� ������ ����
                                         BEEP();
                                         LCD_SetBackColor(RGB_BLUE);     //B���� ���̽�ƽ �Ķ��� �ڽ� �Ͼ� ���ڷ� �ʱ�ȭ
                                         LCD_SetTextColor(RGB_WHITE);
                                         LCD_DisplayChar(3,3,'M');
                                         LCD_DisplayChar(2,2,'S');
					 LCD_SetBackColor(RGB_BLUE);
                                         LCD_SetTextColor(RGB_RED);      //B����
                                         LCD_DisplayChar(3,1,'B');  // subtitle
                                         StickUP_Flag=0;
                                         StickR_Flag=0;
					StickL_Flag = 1;                //LEFT�� ����
				}                          
                                break;
                        case NAVI_RIGHT:	// Joystick RIGHT	
				if (StickR_Flag==0 && RF_Flag==0) { //���̽�ƽ�� RIGHT �����̰� ������ �ʿ����� ������ ����
                                        BEEP();
                                        LCD_SetBackColor(RGB_BLUE);     //M���� ���̽�ƽ �Ķ��� �ڽ� �Ͼ� ���ڷ� �ʱ�ȭ
                                        LCD_SetTextColor(RGB_WHITE);
                                        LCD_DisplayChar(3,1,'B');
                                        LCD_DisplayChar(2,2,'S');
					LCD_SetBackColor(RGB_BLUE);
                                        LCD_SetTextColor(RGB_RED);      //M����
                                        LCD_DisplayChar(3,3,'M');  // subtitle
                                        StickUP_Flag=0;
                                        StickL_Flag=0;
					StickR_Flag = 1;                //RIGHT�� ����
				}
                                break;
                }
                
		switch(KEY_Scan())	// �Էµ� Switch ���� �з� 		
		{
        		case 0xFB00 : 	//SW2 �Է�  WORKING
                        	if(cp>0 && sg>0 && mk>0 && cf>0 && RF_Flag ==0 && ( (StickUP_Flag==1 && CoinInput >= 20) || (StickL_Flag==1 && CoinInput >=10) || (StickR_Flag==1 && CoinInput >= 30)))
                                {               //��� ���� 1���̻� �ְ�, ������ �ʿ����, �ʿ��� ��ŭ ������ �������� WORKING
                                    SW2_Push = 1;               //WORKING ���� ����
                                    BEEP();
                                    GPIOG->ODR |= 0x00FF;               //LED0~7 ON
                                    LCD_SetBackColor(RGB_RED);          //WORKING ����
                                    LCD_SetTextColor(RGB_WHITE);
                                    LCD_DisplayChar(4,2,'0');  // subtitle
                                    DelayMS(1000);
                                    LCD_DisplayChar(4,2,'1');  // subtitle
                                    DelayMS(1000);
                                    LCD_DisplayChar(4,2,'2');  // subtitle
                                    DelayMS(1000);
                                    LCD_DisplayChar(4,2,'W');  // subtitle
                                    GPIOG->ODR &= ~0x00FF;               //LED0~7 OFF
                                    BEEP();
                                    DelayMS(500);
                                    BEEP();
                                    DelayMS(500);
                                    BEEP();
                                    
                                    if(StickUP_Flag==1) //sugar 20 
                                    {
                                        cp--;           //���� Ŀ�� ��� �ϳ��� ����
                                        sg--;
                                        cf--;
                                        CoinInput -= 20;                //20�� ����
                                        Total+=20;                        //20�� ���� ����
                                        LCD_DisplayChar(4,14,Total/100+0x30); //�����ڸ�
                                        LCD_DisplayChar(4,15,(Total%100)/10+0x30); //���� �ڸ�
                                        LCD_DisplayChar(4,16,Total%10+0x30); //�����ڸ�
                                        
                                        StickUP_Flag=0;                 //�� ������ ���̽�ƽ ����
                                    }
                                    if(StickL_Flag==1) //Black 10
                                    {
                                        cp--;           //�� Ŀ�� ��� �ϳ��� ����
                                        cf--;
                                        CoinInput -= 10;                //10�� ����
                                        Total+=10;                        //10�� ���� ����
                                        

                                        StickL_Flag=0;                  //LEFT ���̽�ƽ ����
                                    }
                                    if(StickR_Flag==1) //Mix 30
                                    {
                                        cp--;                   //�ͽ� Ŀ�� ��� �ϳ��� ����
                                        sg--;
                                        mk--;
                                        cf--;
                                        CoinInput -= 30;                //30�� ����
                                        Total+=30;                        //30�� ���� ����
                                        StickR_Flag=0;                  //RIGHT ���̽�ƽ ����
                                    }
                                    NoC++;                     //�Ǹŵ� Ŀ�� �� ����
                                    if(NoC>50)                  //�Ǹ� Ŀ�Ǽ� 50 �ʰ��ϸ� 0���� �ʱ�ȭ
                                      NoC=0;
                                    if(cp==0 || sg==0 || mk==0 || cf==0 )               //��� �� �ϳ��� 0�� �Ǹ� ���� �ʿ� ���·� ǥ��
                                    {
                                      LCD_SetBrushColor(RGB_RED);               //���� �簢�� ������ ǥ��
                                      LCD_DrawFillRect(125,78,10,12);
                                      RF_Flag=1;                                        //���� �ʿ����ǥ��
                                    }
                                    if(Total>990)               //������ 990���� �Ѿ�� 990���� �� ������ ���� ǥ��
                                    {
                                        Total-=990;
                                        

                                    }
                                    LowByte = (unsigned char)(Total & 0x00FF);                  //TOTAL�� ��,���� �ڸ� ����
                                    HighByte = (unsigned char)(Total >> 8 ) & (0x00FF);         //TOTAL�� ���� �ڸ� ����
                                    Fram_Write(625,LowByte);            //625������ LOWBYTE �� ����
                                    Fram_Write(626,HighByte);            //626������ HIGHBYTE �� ����

                                    DisplayInitScreen();                        //���� ������ ó�� ȭ��
                                    SW2_Push = 0;                               //WORKING ���� ����
                                }
                                break;

        	}  // switch(KEY_Scan())
                
                //EXTI Example(1) : SW0�� High���� Low�� �� ��(Falling edge Trigger mode) LED0 toggle
		if(SW0_Flag && RF_Flag==0)	// ���ͷ�Ʈ (EXTI8) �߻�  //10�� ����
		{
			
                        BEEP();
                        CoinInput += 10;                //10�� ����
                        if(CoinInput > 200)             //���Ե� ������ 200�� �Ѿ�� 200�� �� ������ ǥ��
                        {
                            CoinInput-=200;
                        }
                        LCD_SetBackColor(RGB_GRAY);
                        LCD_SetTextColor(RGB_YELLOW);
                        LCD_DisplayChar(2,14,CoinInput/100+0x30); //�����ڸ�
                        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //���� �ڸ�
                        LCD_DisplayChar(2,16,CoinInput%10+0x30); //�����ڸ�
                        
                        LCD_SetBrushColor(RGB_YELLOW); //���� ���Ա� ����
                        LCD_DrawFillRect(76,26,8,10);        
                        DelayMS(500);
                        
                        LCD_SetBrushColor(RGB_GRAY); //���� ���Ա�   ����
                        LCD_DrawFillRect(76,26,8,10);
			SW0_Flag = 0;           //���ͷ�Ʈ Ż��
		}

		//EXTI Example(2) : SW1�� Low���� High�� �� ��(Rising edge Trigger mode) LED1 toggle
		if(SW1_Flag && RF_Flag==0)	// ���ͷ�Ʈ (EXTI9) �߻�  //50�� ����
		{
                        BEEP();
                        CoinInput += 50;               //50�� ����
                        if(CoinInput > 200)             //���Ե� ������ 200�� �Ѿ�� 200�� �� ������ ǥ��
                        {
                            CoinInput-=200;
                        }
                        LCD_SetBackColor(RGB_GRAY);
                        LCD_SetTextColor(RGB_YELLOW);
                        LCD_DisplayChar(2,14,CoinInput/100+0x30); //�����ڸ�
                        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //���� �ڸ�
                        LCD_DisplayChar(2,16,CoinInput%10+0x30); //�����ڸ�
                        
                        LCD_SetBrushColor(RGB_YELLOW); //���� ���Ա� ����
                        LCD_DrawFillRect(76,52,8,10);
                        DelayMS(500);
                        GPIOG->ODR &= ~0x02;
                        LCD_SetBrushColor(RGB_GRAY); //���� ���Ա� ����
                        LCD_DrawFillRect(76,52,8,10);
			SW1_Flag = 0;           //���ͷ�Ʈ Ż��
		}
                if(SW3_Flag)    //���� �ݳ�
                {
                  CoinInput=0;
                  LCD_DisplayChar(2,14,CoinInput/100+0x30); //�����ڸ�
                  LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //���� �ڸ�
                  LCD_DisplayChar(2,16,CoinInput%10+0x30); //�����ڸ�
                  SW3_Flag=0;   //���ͷ�Ʈ Ż��
                }
                if(SW4_Flag && RF_Flag==1)              //���� �ʿ� ���½� SW4�������� ����
                {
                        BEEP();
                        DelayMS(500);
                        BEEP();
                        DelayMS(500);
                        if(cp==0)               //�� ��� �߿� 0�� �Ǵ� ��ᰡ ������ ä����
                          cp=9;
                        if(sg==0)
                          sg=5;
                        if(mk==0)
                          mk=5;
                        if(cf==0)
                          cf=9;
                        
                        RF_Flag=0;              //���� ���� ����
                        DisplayInitScreen();    //ȭ�� �ʱ�ȭ
                        SW4_Flag = 0;           //���ͷ�Ʈ Ż��
                }
    	}  // while(1)
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(Joy stick)) �ʱ� ����	*/
void _GPIO_Init(void)
{
        // LED (GPIO G) ����
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) ���� 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� 
    	RCC->AHB1ENR    |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER    |=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I) ����
	RCC->AHB1ENR    |= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	&=~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR    &=~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)

}	

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) �ʱ� ����  */
void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[2] |= 0x7077; 	// EXTI8,9,11�� ���� �ҽ� �Է��� GPIOH�� ����
					// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])�� �̿� 
					// reset value: 0x0000	
        SYSCFG->EXTICR[3] |= 0x0007;    //EXTI12�� ���� �ҽ� �Է��� GPIOH�� ����

	EXTI->FTSR |= 0x001B00;		// EXTI8,9,11,12: Falling Trigger Enable
    	EXTI->IMR  |= 0x001B00;  	// EXTI8,9,11,12 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[0] |= ( 1 << 23 );   // 0x00800000
        NVIC->ISER[1] |= ( 1 << (40-32) );
					// Enable 'Global Interrupt EXTI8,9,11,12'
					// Vector table Position ����
        
}
void EXTI9_5_IRQHandler(void)		
{
	if(EXTI->PR & 0x0100)                   // EXTI8 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0100; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
		if((SW2_Push == 0)&&(RF_Flag == 0))
                SW0_Flag = 1;			// SW0_Flag: EXTI8�� �߻��Ǿ����� �˸��� ���� ���� ����(main���� mission�� ���)

	}
	else if(EXTI->PR & 0x0200) 		// EXTI9 Interrupt Pending(�߻�) ����?
	{
                EXTI->PR |= 0x0200; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
                if((SW2_Push == 0)&&(RF_Flag == 0))
                SW1_Flag = 1;			// SW1_Flag: EXTI9�� �߻��Ǿ����� �˸��� ���� ���� ����(main���� mission�� ���) 

	}
}
void EXTI15_10_IRQHandler(void)		//EXTI 11,12���� ����
{       
        if (EXTI->PR & 0x0800) 		// EXTI1Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x0800; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
                if((SW2_Push == 0))
		SW3_Flag = 1;                                //: EXTI11�� �߻��Ǿ����� �˸��� ���� ���� ����(main���� mission�� ���) 
                
	}
	else if (EXTI->PR & 0x1000) 		// EXTI2Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x1000; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
		SW4_Flag = 1;                                //: EXTI12�� �߻��Ǿ����� �˸��� ���� ���� ����(main���� mission�� ���) 
                
	}
	
}
/* GLCD �ʱ�ȭ�� ���� */
void DisplayInitScreen(void)
{
        LCD_Clear(RGB_WHITE);		// ȭ�� Ŭ����
        LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
        LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : Green
        LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayText(0,0,"PGW coffee");  // Title
        LCD_SetBackColor(RGB_BLUE);
        LCD_SetTextColor(RGB_WHITE);
        LCD_DisplayChar(2,2,'S');  // subtitle
        LCD_DisplayChar(3,1,'B');  // subtitle
        LCD_DisplayChar(3,3,'M');  // subtitle
        LCD_SetBackColor(RGB_RED);
        LCD_DisplayChar(4,2,'W');  // subtitle
        LCD_SetBackColor(RGB_WHITE);	// ���ڹ��� : Green
        LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayChar(1,8,0x5C); //'\'
        LCD_DisplayText(1,9,"10");  // 10��
        LCD_DisplayChar(3,8,0x5C);//'\'
        LCD_DisplayText(3,9,"50");  // 50��
        LCD_DisplayText(1,14,"IN");
        LCD_DisplayText(3,14,"TOT");
        LCD_SetBrushColor(RGB_GRAY); //���� ���Ա�
        LCD_DrawFillRect(76,26,8,10);
        LCD_DrawFillRect(76,52,8,10);
        LCD_SetBackColor(RGB_GRAY);
        LCD_SetTextColor(RGB_YELLOW);
        LCD_DisplayChar(2,14,CoinInput/100+0x30); //�����ڸ�
        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //���� �ڸ�
        LCD_DisplayChar(2,16,CoinInput%10+0x30); //�����ڸ�
        LowByte = Fram_Read(625);
        HighByte = Fram_Read(626);
        Total = (unsigned int)HighByte;
        Total = (unsigned int)((Total<<8) + LowByte);
        LCD_DisplayChar(4,14,Total /100 +0x30); 
        LCD_DisplayChar(4,15,(Total%100) /10 +0x30); 
        LCD_DisplayChar(4,16,Total%10 +0x30);

        
        LCD_SetBrushColor(RGB_GREEN);
        LCD_DrawFillRect(4,76,16,14);
        LCD_DrawFillRect(28,76,16,14);
        LCD_DrawFillRect(52,76,16,14);
        LCD_DrawFillRect(76,76,16,14);
        LCD_SetBackColor(RGB_GREEN);
        LCD_SetTextColor(RGB_WHITE);
        LCD_DisplayChar(6,1,cp+0x30);
        LCD_DisplayChar(6,4,sg+0x30);
        LCD_DisplayChar(6,7,mk+0x30);
        LCD_DisplayChar(6,10,cf+0x30);
        LCD_SetBackColor(RGB_WHITE);
        LCD_SetTextColor(RGB_BLACK);
        LCD_DisplayText(7,1,"cp sg mk cf");     
        LCD_DisplayText(6,13,"RF");
        if(RF_Flag==0)
            LCD_SetBrushColor(RGB_GREEN);
        if(RF_Flag==1)
            LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(125,78,10,12); //RF

        
        LCD_DisplayText(7,13,"NoC");
        LCD_SetBackColor(RGB_GRAY);
        LCD_SetTextColor(RGB_YELLOW);
        LCD_DisplayChar(8,13,NoC/10+0x30);
        LCD_DisplayChar(8,14,NoC%10+0x30);
        
        
 
}
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick NAVI_* 
{ 
	uint16_t key;
	key = GPIOI->IDR & 0x03E0;	// any key pressed ? 0x0000 0011 1110 0000  masking ����� ���� ���⶧���� ����� ���� ���̱� ���ؼ� ���۾��� ��. 5,6,7,8,9�� �ش�.
	if(key == 0x03E0)		// if no key, check key off
	{  	if(joy_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
        		joy_flag = 0;
        		return key;
        	}
    	}
  	else				// if key input, check continuous key
	{	if(joy_flag != 0)	// if continuous key, treat as no key input
        		return 0x03E0;
      		else			// if new key,delay for debounce
		{	joy_flag = 1;
			DelayMS(10);
 			return key;
        	}
	}
}
/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
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

/* Buzzer: Beep for 30 ms */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(5);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);         		// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
    	for(; Dly; Dly--);
}
