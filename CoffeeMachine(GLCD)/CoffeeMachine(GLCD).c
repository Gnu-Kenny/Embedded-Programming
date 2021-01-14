#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define NAVI_UP		0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_RIGHT	0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);	// Joystick 입력 함수
void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);


unsigned int CoinInput,Total; //코인,수입을 나타내는 변수
unsigned char LowByte,HighByte; //FRAM의 일,십의 자리, 백의 자리의 수를 저장 해줄 변수
uint8_t	SW0_Flag, SW1_Flag,SW3_Flag,SW4_Flag;  //SW0,1,3,4의 상태를 표현할 변수
uint8_t        StickUP_Flag,StickL_Flag,StickR_Flag;            //조이스틱의 상태를 표현할 변수
int cp=9,sg=5,mk=5,cf=9,NoC;                                    //컵,설탕,우유,원두를 나타낼 변수와 판매한 커피 수를 나타내는 변수
int RF_Flag;            //Refill 이 필요한 상태인지를 표현할 변수
uint8_t SW2_Push =0; //인터럽트 중첩을 막을 변수

int main(void)
{
	_GPIO_Init(); 	// GPIO (LED,SW,Buzzer,Joy stick) 초기화
        _EXTI_Init();	// EXTI 초기화
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);

	Fram_Init();                    // FRAM 초기화 H/W 초기화
	Fram_Status_Config();   // FRAM 초기화 S/W 초기화
        LowByte = Fram_Read(625);       //초기화 됐을시 total 값의 십의 자리,일의 자리의 수를 저장
        HighByte = Fram_Read(626);      //초기화 됐을시 total 값의 백의 자리의 수를 저장
        Total = (unsigned int)HighByte;         //백의 자리 저장
        Total = (unsigned int)((Total<<8) + LowByte);//백의 자리를 8비트 시프트하고 십,일의 자리를 저장
        
	DisplayInitScreen();    // LCD 초기화면
//	BEEP();

	GPIOG->ODR &= ~0x00FF;	// LED 초기값: LED0~7 Off
//	LCD_DisplayChar(1,10,Fram_Read(50)+0x30); //FRAM 50번지 저장된 data(1byte) 읽어 LCD에 표시
                                                                   
	while(1)
	{
        
                switch(JOY_Scan())	// 입력된 Joystick 정보 분류
		{
                        case NAVI_UP : 	// Joystick UP
                          if (StickUP_Flag==0 && RF_Flag==0) { //조이스틱이 업 상태이고 리필이 필요하지 않을때 실행
                                        BEEP();
                                        LCD_SetBackColor(RGB_BLUE);     //S외의 조이스틱 파란색 박스 하얀 글자로 초기화
                                        LCD_SetTextColor(RGB_WHITE);
                                        LCD_DisplayChar(3,1,'B');
                                        LCD_DisplayChar(3,3,'M');
                                        LCD_SetBackColor(RGB_BLUE);
                                        LCD_SetTextColor(RGB_RED);      //S선택
                                        LCD_DisplayChar(2,2,'S');
                                        StickR_Flag=0;
					StickL_Flag = 0;
					StickUP_Flag = 1;               //UP인 상태
				}

                                break;
        		
                        case NAVI_LEFT:	// Joystick LEFT	
				if (StickL_Flag==0 && RF_Flag==0) { //조이스틱이 LEFT 상태이고 리필이 필요하지 않을때 실행
                                         BEEP();
                                         LCD_SetBackColor(RGB_BLUE);     //B외의 조이스틱 파란색 박스 하얀 글자로 초기화
                                         LCD_SetTextColor(RGB_WHITE);
                                         LCD_DisplayChar(3,3,'M');
                                         LCD_DisplayChar(2,2,'S');
					 LCD_SetBackColor(RGB_BLUE);
                                         LCD_SetTextColor(RGB_RED);      //B선택
                                         LCD_DisplayChar(3,1,'B');  // subtitle
                                         StickUP_Flag=0;
                                         StickR_Flag=0;
					StickL_Flag = 1;                //LEFT인 상태
				}                          
                                break;
                        case NAVI_RIGHT:	// Joystick RIGHT	
				if (StickR_Flag==0 && RF_Flag==0) { //조이스틱이 RIGHT 상태이고 리필이 필요하지 않을때 실행
                                        BEEP();
                                        LCD_SetBackColor(RGB_BLUE);     //M외의 조이스틱 파란색 박스 하얀 글자로 초기화
                                        LCD_SetTextColor(RGB_WHITE);
                                        LCD_DisplayChar(3,1,'B');
                                        LCD_DisplayChar(2,2,'S');
					LCD_SetBackColor(RGB_BLUE);
                                        LCD_SetTextColor(RGB_RED);      //M선택
                                        LCD_DisplayChar(3,3,'M');  // subtitle
                                        StickUP_Flag=0;
                                        StickL_Flag=0;
					StickR_Flag = 1;                //RIGHT인 상태
				}
                                break;
                }
                
		switch(KEY_Scan())	// 입력된 Switch 정보 분류 		
		{
        		case 0xFB00 : 	//SW2 입력  WORKING
                        	if(cp>0 && sg>0 && mk>0 && cf>0 && RF_Flag ==0 && ( (StickUP_Flag==1 && CoinInput >= 20) || (StickL_Flag==1 && CoinInput >=10) || (StickR_Flag==1 && CoinInput >= 30)))
                                {               //재고가 전부 1개이상 있고, 리필이 필요없고, 필요한 만큼 동전이 들어가있을때 WORKING
                                    SW2_Push = 1;               //WORKING 중인 상태
                                    BEEP();
                                    GPIOG->ODR |= 0x00FF;               //LED0~7 ON
                                    LCD_SetBackColor(RGB_RED);          //WORKING 동작
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
                                        cp--;           //설탕 커피 재료 하나씩 감소
                                        sg--;
                                        cf--;
                                        CoinInput -= 20;                //20원 차감
                                        Total+=20;                        //20원 수입 증가
                                        LCD_DisplayChar(4,14,Total/100+0x30); //백의자리
                                        LCD_DisplayChar(4,15,(Total%100)/10+0x30); //십의 자리
                                        LCD_DisplayChar(4,16,Total%10+0x30); //일의자리
                                        
                                        StickUP_Flag=0;                 //업 상태인 조이스틱 해제
                                    }
                                    if(StickL_Flag==1) //Black 10
                                    {
                                        cp--;           //블랙 커피 재료 하나씩 감소
                                        cf--;
                                        CoinInput -= 10;                //10원 차감
                                        Total+=10;                        //10원 수입 증가
                                        

                                        StickL_Flag=0;                  //LEFT 조이스틱 해제
                                    }
                                    if(StickR_Flag==1) //Mix 30
                                    {
                                        cp--;                   //믹스 커피 재료 하나씩 감소
                                        sg--;
                                        mk--;
                                        cf--;
                                        CoinInput -= 30;                //30원 차감
                                        Total+=30;                        //30원 수입 증가
                                        StickR_Flag=0;                  //RIGHT 조이스틱 해제
                                    }
                                    NoC++;                     //판매된 커피 수 증가
                                    if(NoC>50)                  //판매 커피수 50 초과하면 0으로 초기화
                                      NoC=0;
                                    if(cp==0 || sg==0 || mk==0 || cf==0 )               //재료 중 하나라도 0이 되면 리필 필요 상태로 표현
                                    {
                                      LCD_SetBrushColor(RGB_RED);               //리필 사각형 빨간색 표시
                                      LCD_DrawFillRect(125,78,10,12);
                                      RF_Flag=1;                                        //리필 필요상태표현
                                    }
                                    if(Total>990)               //수입이 990원을 넘어설시 990원을 뺀 나머지 값을 표현
                                    {
                                        Total-=990;
                                        

                                    }
                                    LowByte = (unsigned char)(Total & 0x00FF);                  //TOTAL의 일,십의 자리 저장
                                    HighByte = (unsigned char)(Total >> 8 ) & (0x00FF);         //TOTAL의 백의 자리 저장
                                    Fram_Write(625,LowByte);            //625번지에 LOWBYTE 값 저장
                                    Fram_Write(626,HighByte);            //626번지에 HIGHBYTE 값 저장

                                    DisplayInitScreen();                        //값들 저장후 처음 화면
                                    SW2_Push = 0;                               //WORKING 동작 종료
                                }
                                break;

        	}  // switch(KEY_Scan())
                
                //EXTI Example(1) : SW0가 High에서 Low가 될 때(Falling edge Trigger mode) LED0 toggle
		if(SW0_Flag && RF_Flag==0)	// 인터럽트 (EXTI8) 발생  //10원 투입
		{
			
                        BEEP();
                        CoinInput += 10;                //10원 투입
                        if(CoinInput > 200)             //투입된 동전이 200원 넘어가면 200을 뺀 나머지 표시
                        {
                            CoinInput-=200;
                        }
                        LCD_SetBackColor(RGB_GRAY);
                        LCD_SetTextColor(RGB_YELLOW);
                        LCD_DisplayChar(2,14,CoinInput/100+0x30); //백의자리
                        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //십의 자리
                        LCD_DisplayChar(2,16,CoinInput%10+0x30); //일의자리
                        
                        LCD_SetBrushColor(RGB_YELLOW); //코인 투입기 동작
                        LCD_DrawFillRect(76,26,8,10);        
                        DelayMS(500);
                        
                        LCD_SetBrushColor(RGB_GRAY); //코인 투입기   종료
                        LCD_DrawFillRect(76,26,8,10);
			SW0_Flag = 0;           //인터럽트 탈출
		}

		//EXTI Example(2) : SW1가 Low에서 High가 될 때(Rising edge Trigger mode) LED1 toggle
		if(SW1_Flag && RF_Flag==0)	// 인터럽트 (EXTI9) 발생  //50원 투입
		{
                        BEEP();
                        CoinInput += 50;               //50원 투입
                        if(CoinInput > 200)             //투입된 동전이 200원 넘어가면 200을 뺀 나머지 표시
                        {
                            CoinInput-=200;
                        }
                        LCD_SetBackColor(RGB_GRAY);
                        LCD_SetTextColor(RGB_YELLOW);
                        LCD_DisplayChar(2,14,CoinInput/100+0x30); //백의자리
                        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //십의 자리
                        LCD_DisplayChar(2,16,CoinInput%10+0x30); //일의자리
                        
                        LCD_SetBrushColor(RGB_YELLOW); //코인 투입기 동작
                        LCD_DrawFillRect(76,52,8,10);
                        DelayMS(500);
                        GPIOG->ODR &= ~0x02;
                        LCD_SetBrushColor(RGB_GRAY); //코인 투입기 종료
                        LCD_DrawFillRect(76,52,8,10);
			SW1_Flag = 0;           //인터럽트 탈출
		}
                if(SW3_Flag)    //동전 반납
                {
                  CoinInput=0;
                  LCD_DisplayChar(2,14,CoinInput/100+0x30); //백의자리
                  LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //십의 자리
                  LCD_DisplayChar(2,16,CoinInput%10+0x30); //일의자리
                  SW3_Flag=0;   //인터럽트 탈출
                }
                if(SW4_Flag && RF_Flag==1)              //리필 필요 상태시 SW4눌렀을때 동작
                {
                        BEEP();
                        DelayMS(500);
                        BEEP();
                        DelayMS(500);
                        if(cp==0)               //각 재료 중에 0이 되는 재료가 있으면 채워줌
                          cp=9;
                        if(sg==0)
                          sg=5;
                        if(mk==0)
                          mk=5;
                        if(cf==0)
                          cf=9;
                        
                        RF_Flag=0;              //리필 동작 종료
                        DisplayInitScreen();    //화면 초기화
                        SW4_Flag = 0;           //인터럽트 탈출
                }
    	}  // while(1)
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

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[2] |= 0x7077; 	// EXTI8,9,11에 대한 소스 입력은 GPIOH로 설정
					// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])를 이용 
					// reset value: 0x0000	
        SYSCFG->EXTICR[3] |= 0x0007;    //EXTI12에 대한 소스 입력은 GPIOH로 설정

	EXTI->FTSR |= 0x001B00;		// EXTI8,9,11,12: Falling Trigger Enable
    	EXTI->IMR  |= 0x001B00;  	// EXTI8,9,11,12 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1 << 23 );   // 0x00800000
        NVIC->ISER[1] |= ( 1 << (40-32) );
					// Enable 'Global Interrupt EXTI8,9,11,12'
					// Vector table Position 참조
        
}
void EXTI9_5_IRQHandler(void)		
{
	if(EXTI->PR & 0x0100)                   // EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if((SW2_Push == 0)&&(RF_Flag == 0))
                SW0_Flag = 1;			// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용)

	}
	else if(EXTI->PR & 0x0200) 		// EXTI9 Interrupt Pending(발생) 여부?
	{
                EXTI->PR |= 0x0200; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                if((SW2_Push == 0)&&(RF_Flag == 0))
                SW1_Flag = 1;			// SW1_Flag: EXTI9가 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 

	}
}
void EXTI15_10_IRQHandler(void)		//EXTI 11,12쓰기 위함
{       
        if (EXTI->PR & 0x0800) 		// EXTI1Interrupt Pending(발생) 여부
	{
		EXTI->PR |= 0x0800; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
                if((SW2_Push == 0))
		SW3_Flag = 1;                                //: EXTI11가 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                
	}
	else if (EXTI->PR & 0x1000) 		// EXTI2Interrupt Pending(발생) 여부
	{
		EXTI->PR |= 0x1000; 		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		SW4_Flag = 1;                                //: EXTI12가 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
                
	}
	
}
/* GLCD 초기화면 설정 */
void DisplayInitScreen(void)
{
        LCD_Clear(RGB_WHITE);		// 화면 클리어
        LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
        LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Green
        LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayText(0,0,"PGW coffee");  // Title
        LCD_SetBackColor(RGB_BLUE);
        LCD_SetTextColor(RGB_WHITE);
        LCD_DisplayChar(2,2,'S');  // subtitle
        LCD_DisplayChar(3,1,'B');  // subtitle
        LCD_DisplayChar(3,3,'M');  // subtitle
        LCD_SetBackColor(RGB_RED);
        LCD_DisplayChar(4,2,'W');  // subtitle
        LCD_SetBackColor(RGB_WHITE);	// 글자배경색 : Green
        LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
        LCD_DisplayChar(1,8,0x5C); //'\'
        LCD_DisplayText(1,9,"10");  // 10원
        LCD_DisplayChar(3,8,0x5C);//'\'
        LCD_DisplayText(3,9,"50");  // 50원
        LCD_DisplayText(1,14,"IN");
        LCD_DisplayText(3,14,"TOT");
        LCD_SetBrushColor(RGB_GRAY); //코인 투입기
        LCD_DrawFillRect(76,26,8,10);
        LCD_DrawFillRect(76,52,8,10);
        LCD_SetBackColor(RGB_GRAY);
        LCD_SetTextColor(RGB_YELLOW);
        LCD_DisplayChar(2,14,CoinInput/100+0x30); //백의자리
        LCD_DisplayChar(2,15,(CoinInput%100)/10+0x30); //십의 자리
        LCD_DisplayChar(2,16,CoinInput%10+0x30); //일의자리
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
	key = GPIOI->IDR & 0x03E0;	// any key pressed ? 0x0000 0011 1110 0000  masking 경우의 수가 많기때문에 경우의 수를 줄이기 위해서 이작업을 함. 5,6,7,8,9에 해당.
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
