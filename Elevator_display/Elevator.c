#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#define RGB_VIOLET      GET_RGB(250,0,250)

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15


void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

unsigned char Fl_Cur, Fl_6th; //���� ��, 6�� ����
uint8_t	SW7_Flag; //���ͷ�Ʈ �߻����� �Ǵ� ����
uint8_t	Elevator_Flag=1,Elevator_Init=1; // ��ǥ ��, ���� ��
int i,flag=0,flag_init=0;  //���º� ���� ������ ���� flag���� ����
char a='1'; //�� ��
int main(void)
{
	_GPIO_Init(); 	// GPIO (LED,SW,Buzzer,Joy stick) �ʱ�ȭ
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);

	Fram_Init();                    // FRAM �ʱ�ȭ H/W �ʱ�ȭ
	Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
        
	DisplayInitScreen();    // LCD �ʱ�ȭ��
//	BEEP();

	GPIOG->ODR &= ~0x00FF;	// LED �ʱⰪ: LED0~7 Off
        Fl_Cur=1; //������ 
        Fl_6th=Fram_Read(610);          //610���� ����� data(1byte) �о� 
	LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610���� ����� data(1byte) �о� LCD�� ǥ��
        _EXTI_Init();
	while(1)
	{
		switch(KEY_Scan())	// �Էµ� Switch ���� �з� 		
		{
        		case SW0_PUSH : 	//SW0 �Է�
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'B');  ///////B������ �ٲܰ�
                                a='B'; //������
                                Elevator_Flag=0; //������ ����� ���� flag
                                GPIOG->ODR = 0x0001;  //LED0 ON
                                BEEP();
                                break;
        		case SW1_PUSH : 	//SW1 �Է�
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'1');  ///////1������ �ٲܰ�
                                a='1';  //������
                                Elevator_Flag=1;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0002;  //LED1 ON                            
                                BEEP();
                                break;
        		case SW2_PUSH : 	//SW2 �Է�
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'2');  ///////2������ �ٲܰ�
                                a='2';  //������
                                Elevator_Flag=2;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0004;  //LED2 ON       
                                BEEP();
                                break;
        		case SW3_PUSH : 	//SW3 �Է�                      
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'3');  ///////3������ �ٲܰ�
                                a='3';  //������
                                Elevator_Flag=3;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0008;  //LED3 ON                 
                                BEEP();                                                                                                
                                break;
                        case SW4_PUSH : 	//SW4 �Է½�     
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'4');  ///////4������ �ٲܰ�
                                a='4';  //������
                                Elevator_Flag=4;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0010;  //LED4 ON       
                                BEEP();
                                break;
                        case SW5_PUSH : 	//SW5 �Է½�
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'5');  ///////5������ �ٲܰ�
                                a='5';  //������
                                Elevator_Flag=5;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0020;  //LED5 ON
                                BEEP();
                                break;
                        case SW6_PUSH : 	//SW6 �Է�    
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'6');  ///////6������ �ٲܰ�
                                a='6';  //������
                                Elevator_Flag=6;  //������ ����� ���� flag
                                GPIOG->ODR = 0x0040;  //LED6 ON
                                BEEP();
                                break;
        	}  // switch(KEY_Scan())
                if (SW7_Flag)	                                              // ���ͷ�Ʈ (EXTI15) �߻�
		{
			
//                      BEEP();                                                    //  Buzzer
                       
                        if(Elevator_Flag-Elevator_Init>0) // ���
                        {
                            GPIOG->ODR |= 0x0080;	                // LED7 ON
                            LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_RED); // �� �� : Red
                                LCD_DrawHorLine(13, 19, 10); //��� ȭ��ǥ
                                LCD_DrawLine(23,19, 20,15);
                                LCD_DrawLine(23,19, 20,23);
                                BEEP();
                                DelayMS(500);
                                BEEP();
                            for(i=0;i<=Elevator_Flag-Elevator_Init;i++)//���̳��� ������ŭ �����
                            {
                              DisplayInitScreen(); //����� ������ ȭ�� �ʱ�ȭ
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610���� ����� data(1byte) �о� LCD�� ǥ��          
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_RED); // �� �� : Red
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(23,19, 20,15);
                                LCD_DrawLine(23,19, 20,23);
                                LCD_SetPenColor(RGB_GREEN);
                                LCD_DrawRectangle(56+16*(Elevator_Init-1), 40, 8+16*i, 8);        // ��й�ȣ �Է� ���¸� ��Ÿ���� ������ �׵θ� �簢�� ���  x y   B(40,40,8,8) 1(56,40,8,8) 2(72,40,8,8)
                                LCD_SetBrushColor(RGB_RED);
                                LCD_DrawFillRect(57+16*(Elevator_Init-1),41,7+16*i,7);
                                DelayMS(500);
                            }
                            if(Elevator_Flag==6) //6�� ���� 
                            {
                                Fl_6th++;  // 6���� ����Ƚ�� ����
                                if(Fl_6th>=10) //10�̻�� 0���� �ʱ�ȭ
                                  Fl_6th=0; 
                                Fram_Write(610,Fl_6th);  // 610���� FRAM�� ����
                                LCD_SetTextColor(RGB_VIOLET);	// ���ڻ� :  �����
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610���� ����� data(1byte) �о� LCD�� ǥ��
                            }
                            Elevator_Init=Elevator_Flag; //������ ���� ����
                            LCD_SetTextColor(RGB_BLACK);
                            LCD_DisplayText(1,0,"S   ");  // subtitle
                            LCD_SetTextColor(RGB_VIOLET);	// ���ڻ� :  �����
                            LCD_DisplayChar(1,2,'O'); 
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            GPIOG->ODR &= ~0x00FF; //��� LED OFF
                            SW7_Flag = 0;                                           // if (SW7_Flag) Ż��
                        }
                        else if(Elevator_Flag-Elevator_Init<0) //�ϰ�
                        {
                            GPIOG->ODR |= 0x0080;	                               // LED7 ON
                            LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_BLUE); //�ϰ� ȭ��ǥ
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(16,15, 13,19);
                                LCD_DrawLine(13,19, 16,23);
                                BEEP();
                                DelayMS(500);
                                BEEP();
                            for(i=0;i<=Elevator_Init-Elevator_Flag;i++) //���̳��� ������ŭ �����
                            {
                                DisplayInitScreen();  //����� ������ ȭ�� �ʱ�ȭ
                        
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610���� ����� data(1byte) �о� LCD�� ǥ��
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_BLUE); //�ϰ� ȭ��ǥ
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(16,15, 13,19);
                                LCD_DrawLine(13,19, 16,23);
                                
                                LCD_SetPenColor(RGB_GREEN); //BAR �׵θ�
                                LCD_DrawRectangle((56+16*(Elevator_Init-1))-16*i, 40, 8+16*i, 8);     //���̳��� ������ŭ �����
                                LCD_SetBrushColor(RGB_BLUE); //�ϰ� BAR
                                LCD_DrawFillRect((57+16*(Elevator_Init-1))-16*i,41,7+16*i,7);           //���̳��� ������ŭ �����
                                DelayMS(500);
                            }
                            
                            Elevator_Init=Elevator_Flag; //������ 
                            LCD_SetTextColor(RGB_BLACK);
                            LCD_DisplayText(1,0,"S   ");  // subtitle
                            LCD_SetTextColor(RGB_VIOLET);	// ���ڻ� :  �����
                            LCD_DisplayChar(1,2,'O');
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            GPIOG->ODR &= ~0x00FF;  //��� LED OFF
                            SW7_Flag = 0;                                           // if (SW7_Flag) Ż��
                        }
                        else //���� ���� ������ ������ �︮�� Ż���Ŵ
                        {
                          BEEP();
                          SW7_Flag = 0; 
                        }
		}
      
      }
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


void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[3] |= 0x7000; 	// EXTI15�� ���� �ҽ� �Է��� GPIOH�� ����
					

	EXTI->FTSR |= 0x008000;		// EXTI15: Falling Trigger Enable
    	EXTI->IMR  |= 0x008000;  	// EXTI15 ���ͷ�Ʈ mask (Interrupt Enable) ����
		
	NVIC->ISER[1] |= ( 1 << (40-32) );   // 0x00800000
					// Enable 'Global Interrupt EXTI15'
					// Vector table Position ����
}


void EXTI15_10_IRQHandler(void)		
{
	if (EXTI->PR & 0x8000) 		// EXTI5Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x8000; 		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
		SW7_Flag = 1;                                //: EXTI12�� �߻��Ǿ����� �˸��� ���� ���� ����(main���� mission�� ���) 
                
//		EXTI->IMR &= ~0x008000;    //���� ���ͷ�Ʈ mask ���� (EXTI15)���� 
	}
	
}
/* GLCD �ʱ�ȭ�� ���� */
void DisplayInitScreen(void)
{
        LCD_Clear(RGB_YELLOW);		// ȭ�� Ŭ����
        LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
        LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : YELLOW
        LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
        LCD_DisplayText(0,0,"2015132012 PGW");  // Title
        LCD_DisplayText(1,0,"S    6No:");  // subtitle
        LCD_DisplayText(2,0,"FL:"); // subtitle
        LCD_DisplayChar(2,3,a); // subtitle

        LCD_DisplayText(2,5,"B 1 2 3 4 5 6");  // subtitle
        LCD_SetTextColor(RGB_VIOLET);	// ���ڻ� :  �����
        LCD_DisplayChar(1,2,'O');
        
        LCD_SetPenColor(RGB_GREEN); //bar �׵θ�
	LCD_DrawRectangle(56+16*(Elevator_Init-1), 40, 8, 8);   
        LCD_SetBrushColor(RGB_RED); //�ʱ� bar
        LCD_DrawFillRect(57+16*(Elevator_Init-1),41,7,7);
	
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
	DelayMS(10);		// Delay 10 ms
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
