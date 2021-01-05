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

unsigned char Fl_Cur, Fl_6th; //현재 층, 6층 도달
uint8_t	SW7_Flag; //인터럽트 발생여부 판단 변수
uint8_t	Elevator_Flag=1,Elevator_Init=1; // 목표 층, 현재 층
int i,flag=0,flag_init=0;  //상태별 동작 구분을 위한 flag변수 선언
char a='1'; //층 수
int main(void)
{
	_GPIO_Init(); 	// GPIO (LED,SW,Buzzer,Joy stick) 초기화
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);

	Fram_Init();                    // FRAM 초기화 H/W 초기화
	Fram_Status_Config();   // FRAM 초기화 S/W 초기화
        
	DisplayInitScreen();    // LCD 초기화면
//	BEEP();

	GPIOG->ODR &= ~0x00FF;	// LED 초기값: LED0~7 Off
        Fl_Cur=1; //현재층 
        Fl_6th=Fram_Read(610);          //610번지 저장된 data(1byte) 읽어 
	LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610번지 저장된 data(1byte) 읽어 LCD에 표시
        _EXTI_Init();
	while(1)
	{
		switch(KEY_Scan())	// 입력된 Switch 정보 분류 		
		{
        		case SW0_PUSH : 	//SW0 입력
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'B');  ///////B층으로 바꿀것
                                a='B'; //현재층
                                Elevator_Flag=0; //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0001;  //LED0 ON
                                BEEP();
                                break;
        		case SW1_PUSH : 	//SW1 입력
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'1');  ///////1층으로 바꿀것
                                a='1';  //현재층
                                Elevator_Flag=1;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0002;  //LED1 ON                            
                                BEEP();
                                break;
        		case SW2_PUSH : 	//SW2 입력
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'2');  ///////2층으로 바꿀것
                                a='2';  //현재층
                                Elevator_Flag=2;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0004;  //LED2 ON       
                                BEEP();
                                break;
        		case SW3_PUSH : 	//SW3 입력                      
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayChar(2,3,'3');  ///////3층으로 바꿀것
                                a='3';  //현재층
                                Elevator_Flag=3;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0008;  //LED3 ON                 
                                BEEP();                                                                                                
                                break;
                        case SW4_PUSH : 	//SW4 입력시     
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'4');  ///////4층으로 바꿀것
                                a='4';  //현재층
                                Elevator_Flag=4;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0010;  //LED4 ON       
                                BEEP();
                                break;
                        case SW5_PUSH : 	//SW5 입력시
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'5');  ///////5층으로 바꿀것
                                a='5';  //현재층
                                Elevator_Flag=5;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0020;  //LED5 ON
                                BEEP();
                                break;
                        case SW6_PUSH : 	//SW6 입력    
                                LCD_SetTextColor(RGB_BLACK);	
                                LCD_DisplayChar(2,3,'6');  ///////6층으로 바꿀것
                                a='6';  //현재층
                                Elevator_Flag=6;  //현재층 계산을 위한 flag
                                GPIOG->ODR = 0x0040;  //LED6 ON
                                BEEP();
                                break;
        	}  // switch(KEY_Scan())
                if (SW7_Flag)	                                              // 인터럽트 (EXTI15) 발생
		{
			
//                      BEEP();                                                    //  Buzzer
                       
                        if(Elevator_Flag-Elevator_Init>0) // 상승
                        {
                            GPIOG->ODR |= 0x0080;	                // LED7 ON
                            LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_RED); // 펜 색 : Red
                                LCD_DrawHorLine(13, 19, 10); //상승 화살표
                                LCD_DrawLine(23,19, 20,15);
                                LCD_DrawLine(23,19, 20,23);
                                BEEP();
                                DelayMS(500);
                                BEEP();
                            for(i=0;i<=Elevator_Flag-Elevator_Init;i++)//차이나는 층수만큼 재출력
                            {
                              DisplayInitScreen(); //변경된 정보로 화면 초기화
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610번지 저장된 data(1byte) 읽어 LCD에 표시          
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_RED); // 펜 색 : Red
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(23,19, 20,15);
                                LCD_DrawLine(23,19, 20,23);
                                LCD_SetPenColor(RGB_GREEN);
                                LCD_DrawRectangle(56+16*(Elevator_Init-1), 40, 8+16*i, 8);        // 비밀번호 입력 상태를 나타내는 검은색 테두리 사각형 출력  x y   B(40,40,8,8) 1(56,40,8,8) 2(72,40,8,8)
                                LCD_SetBrushColor(RGB_RED);
                                LCD_DrawFillRect(57+16*(Elevator_Init-1),41,7+16*i,7);
                                DelayMS(500);
                            }
                            if(Elevator_Flag==6) //6층 도달 
                            {
                                Fl_6th++;  // 6층에 도착횟수 증가
                                if(Fl_6th>=10) //10이상시 0으로 초기화
                                  Fl_6th=0; 
                                Fram_Write(610,Fl_6th);  // 610번지 FRAM에 저장
                                LCD_SetTextColor(RGB_VIOLET);	// 글자색 :  보라색
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610번지 저장된 data(1byte) 읽어 LCD에 표시
                            }
                            Elevator_Init=Elevator_Flag; //현재층 정보 저장
                            LCD_SetTextColor(RGB_BLACK);
                            LCD_DisplayText(1,0,"S   ");  // subtitle
                            LCD_SetTextColor(RGB_VIOLET);	// 글자색 :  보라색
                            LCD_DisplayChar(1,2,'O'); 
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            GPIOG->ODR &= ~0x00FF; //모든 LED OFF
                            SW7_Flag = 0;                                           // if (SW7_Flag) 탈출
                        }
                        else if(Elevator_Flag-Elevator_Init<0) //하강
                        {
                            GPIOG->ODR |= 0x0080;	                               // LED7 ON
                            LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_BLUE); //하강 화살표
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(16,15, 13,19);
                                LCD_DrawLine(13,19, 16,23);
                                BEEP();
                                DelayMS(500);
                                BEEP();
                            for(i=0;i<=Elevator_Init-Elevator_Flag;i++) //차이나는 층수만큼 재출력
                            {
                                DisplayInitScreen();  //변경된 정보로 화면 초기화
                        
                                LCD_DisplayChar(1,10,Fram_Read(610)+0x30); //FRAM 610번지 저장된 data(1byte) 읽어 LCD에 표시
                                LCD_SetTextColor(RGB_BLACK);
                                LCD_DisplayText(1,0,"M   ");  // subtitle
                                LCD_SetPenColor(RGB_BLUE); //하강 화살표
                                LCD_DrawHorLine(13, 19, 10);
                                LCD_DrawLine(16,15, 13,19);
                                LCD_DrawLine(13,19, 16,23);
                                
                                LCD_SetPenColor(RGB_GREEN); //BAR 테두리
                                LCD_DrawRectangle((56+16*(Elevator_Init-1))-16*i, 40, 8+16*i, 8);     //차이나는 층수만큼 재출력
                                LCD_SetBrushColor(RGB_BLUE); //하강 BAR
                                LCD_DrawFillRect((57+16*(Elevator_Init-1))-16*i,41,7+16*i,7);           //차이나는 층수만큼 재출력
                                DelayMS(500);
                            }
                            
                            Elevator_Init=Elevator_Flag; //현재층 
                            LCD_SetTextColor(RGB_BLACK);
                            LCD_DisplayText(1,0,"S   ");  // subtitle
                            LCD_SetTextColor(RGB_VIOLET);	// 글자색 :  보라색
                            LCD_DisplayChar(1,2,'O');
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            DelayMS(500);
                            BEEP();
                            GPIOG->ODR &= ~0x00FF;  //모든 LED OFF
                            SW7_Flag = 0;                                           // if (SW7_Flag) 탈출
                        }
                        else //같은 층을 누를시 부저만 울리고 탈출시킴
                        {
                          BEEP();
                          SW7_Flag = 0; 
                        }
		}
      
      }
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(Joy stick)) 초기 설정	*/
void _GPIO_Init(void)
{
        // LED (GPIO G) 설정
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
    	RCC->AHB1ENR    |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER    |=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I) 설정
	RCC->AHB1ENR    |= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	&=~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR    &=~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)

}	


void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[3] |= 0x7000; 	// EXTI15에 대한 소스 입력은 GPIOH로 설정
					

	EXTI->FTSR |= 0x008000;		// EXTI15: Falling Trigger Enable
    	EXTI->IMR  |= 0x008000;  	// EXTI15 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[1] |= ( 1 << (40-32) );   // 0x00800000
					// Enable 'Global Interrupt EXTI15'
					// Vector table Position 참조
}


void EXTI15_10_IRQHandler(void)		
{
	if (EXTI->PR & 0x8000) 		// EXTI5Interrupt Pending(발생) 여부
	{
		EXTI->PR |= 0x8000; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		SW7_Flag = 1;                                //: EXTI12가 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                
//		EXTI->IMR &= ~0x008000;    //현재 인터럽트 mask 설정 (EXTI15)해제 
	}
	
}
/* GLCD 초기화면 설정 */
void DisplayInitScreen(void)
{
        LCD_Clear(RGB_YELLOW);		// 화면 클리어
        LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
        LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : YELLOW
        LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayText(0,0,"2015132012 PGW");  // Title
        LCD_DisplayText(1,0,"S    6No:");  // subtitle
        LCD_DisplayText(2,0,"FL:"); // subtitle
        LCD_DisplayChar(2,3,a); // subtitle

        LCD_DisplayText(2,5,"B 1 2 3 4 5 6");  // subtitle
        LCD_SetTextColor(RGB_VIOLET);	// 글자색 :  보라색
        LCD_DisplayChar(1,2,'O');
        
        LCD_SetPenColor(RGB_GREEN); //bar 테두리
	LCD_DrawRectangle(56+16*(Elevator_Init-1), 40, 8, 8);   
        LCD_SetBrushColor(RGB_RED); //초기 bar
        LCD_DrawFillRect(57+16*(Elevator_Init-1),41,7,7);
	
}





/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
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
