/*
 * bib.h
 *
 *  Created on: Jan 8, 2025
 *      Author: Andrzej Ozga
 *      Wt tej biblitece są wszystkie funkcjie i struktury użyte w Projekcie pt. "Autko"
 */

#ifndef INC_BIB_H_
#define INC_BIB_H_
#include <stdlib.h>
typedef enum //tworzę nowy typ zmiennej enumerycznej-MOTOR_STATE_Type - czyli stan w jakim znajduje sie dane koło
{
	IDLE_1=0,              //to stan jałowy
	DRIVE,                 //to stan jazdy
	BRAKE                  //to stan hamowania
 }MOTOR_STATE_Type;

 typedef enum //tworzę nowy typ zmiennej-MOVE_STATE_Type czyli sposob w jaki auto sie porusza
 {
 	IDLE_2= 0,             //to stan nie ustalony
 	FORWARD,               //to stan jazdy do przodu/tyłu
 	RIGHT_SIDE,            //to stan jazdy w stronę prawą/lewą
 	//DIAGONAL_P,            //to stan jazdy po ukosie w prawą stronę do przodu/tyłu
 	//DIAGONAL_L,            //to stan jazdy po ukosie w lewą strone do przodu/tyłu
	AROUND,                //to stan jazdy wokół wlasnej osi
 	//AROUND_F,              //to stan jazdy gdzie obracamy sie wokoł osi z przodu
	//AROUND_B,              //to stan jazdy gdzie obracamy sie wokoł osi z tyłu
	//AROUND_R,              //to stan jazdy gdzie obracamy sie wokoł osi z prawej strony
	//AROUND_L               //to stan jazdy gdzie obracamy sie wokoł osi z lewj strony
 }MOVE_STATE_Type;

 typedef struct //toworze strukture-ENCODER_Type sa tutaj wszytkie parametry dla enkodera
  {
 	 int Value;             //tworzę zmienną która pamięta ostanią wartość
 	 uint32_t TotalTurns;        //tworze zmienną która przechowuje wszytkie obroty które zrobił silnik
 	 float CurrentVelocity;      //tworze zmienną ktora przechowuje aktualną prędkosc  znak to kierunek obrotów
 	 float V1;                   //to wartości predkosci o 1,2,3 próbki wcześniej będą słuzyć do średniej aby wyelminować skoki prędkosci
 	 // tutaj bedą jeszce inne parametry które można obliczyć z enkdera
  }ENCODER_Type;

  typedef struct //toworze strukture-TIMER_Type sa tutaj wszytkie numery timerów i kanały
    {
  	 TIM_HandleTypeDef     *_htim_PWM;           //to wskaźnik na timer od pwma
  	 uint32_t              _channel_PWM;         //to wskaźnik na kanał od pwma
    }TIMER_Type;

   typedef struct //toworze strukture-PINOUT_Type sa tutaj wszytkie numery pinów i portów dla jednego koła
   {
	   GPIO_TypeDef*       DirectionPort;     //to wskaźnik na port od kierunku jazdy
       uint16_t            DirectionPin;      //to pin od kierunku jazdy

       GPIO_TypeDef*       EncoderPort;     //to wskaźnik na port od enkodera
       uint16_t            EncoderPin;      //to pin od enkodera
   }PINOUT_Type;

   typedef struct //tworzę strukture PID_Type są tutaj wszystkie dane potrzebne do reg. PID
   {
       float 			   PidSignal;     //to syganł obliczony przez nas regulator PID
       float               Deviation;     //to uchyb regulacji - wartość zadana - wartość bieżąca
       float               Integral;      //to całka z uchybu
       float               Derivative;    //to pochodna z uchybu
   }PID_Type;

   typedef struct //tworze strukture RAMP_Type są tutaj wszystkie dane potrzebne do obliczania rampy
   {
   	  int          StartSpeed;         //to prędkość początkowa
   	  int  		   FinalSpeed;         //to predkosć do której chcę dojść
   	  int		   LastFinalSpeed;     //to ostatnia predkosc do której chcieliśmy dojść
   	  float        Delta;              //to przyrost predkosci z kolejnym krokiem
   	  float 	   RisingTime;         //to czas narastania w sekundach
   }RAMP_Type;

typedef struct //toworze strukture-WHEEL_Type sa tutaj wszytkie parametry kola
{
	MOTOR_STATE_Type      MOTOR_STATE;      //to stan silnika
	MOVE_STATE_Type       MOVE_STATE;       //to stan poruszania sie autka
	ENCODER_Type          encoder_data;     //to struktura w której są wszystkie zmienne potrzebne do obsługi enkodera
	TIMER_Type       	  timer_data;       //to struktura w której mam wszystkie kanały i timery
	PINOUT_Type           pinout_data;      //to struktura w której mam wszystkie piny i porty potrzebne do koła
	PID_Type			  pid_data;         //to struktura w której mam wszystkie dane od reg. pid
	RAMP_Type			  ramp_data;        //to struktura w której mam wszystkie dane do rampy prędkosci

	int 				  set_velocity;     //to predkość jaką ustawiam
	int                   motor_siganl;     //to sygnal jaki jest ustawiony na silniku
}WHEEL_Type;

typedef struct      //tworzę strukturę w której mam informację z danego kanału
{
	float     		Voltage;
	float 			Current;
}CHANNEL_Type;

typedef struct     //tworzę strukturę w której mam wszystkie dane z czujnika INA3221 - trzy kanałowego
{
	CHANNEL_Type	Channel1;  //to kanał od obw. ster  - 5V
	CHANNEL_Type    Channel2;  //to kanał od obw. mocy  - 12V
	CHANNEL_Type    Channel3;  //to kanał od baterii    - 16V
	float           Power;     //to moc jaką pobieramy z baterii - jednostki to WATY
	float 			TotalEnergy; //to całkowita zużyta energia - jednostki to Wh
}INA3221_Type;

void InitFunction(WHEEL_Type* FL,WHEEL_Type* FR,WHEEL_Type* BL,WHEEL_Type* BR)//funkcja incalizujaca 4 obiekty
{
	FL->timer_data._htim_PWM=&htim3;
	FL->timer_data._channel_PWM=TIM_CHANNEL_2;

	FR->timer_data._htim_PWM=&htim3;
	FR->timer_data._channel_PWM=TIM_CHANNEL_1;

	BL->timer_data._htim_PWM=&htim3;
	BL->timer_data._channel_PWM=TIM_CHANNEL_3;

	BR->timer_data._htim_PWM=&htim3;
	BR->timer_data._channel_PWM=TIM_CHANNEL_4;

	//to piny od enkdera
	FL->pinout_data.EncoderPin=EN_1_Pin;
	FL->pinout_data.EncoderPort=EN_1_GPIO_Port;

	FR->pinout_data.EncoderPin=EN_2_Pin;
	FR->pinout_data.EncoderPort=EN_2_GPIO_Port;

	BL->pinout_data.EncoderPin=EN_3_Pin;
	BL->pinout_data.EncoderPort=EN_3_GPIO_Port;

	BR->pinout_data.EncoderPin=EN_4_Pin;
	BR->pinout_data.EncoderPort=EN_4_GPIO_Port;

	//to piny i porty od kierunku
	FL->pinout_data.DirectionPin=KIER_1_Pin;
	FL->pinout_data.DirectionPort=KIER_1_GPIO_Port;

	FR->pinout_data.DirectionPin=KIER_2_Pin;
	FR->pinout_data.DirectionPort=KIER_2_GPIO_Port;

	BL->pinout_data.DirectionPin=KIER_3_Pin;
	BL->pinout_data.DirectionPort=KIER_3_GPIO_Port;

	BR->pinout_data.DirectionPin=KIER_4_Pin;
	BR->pinout_data.DirectionPort=KIER_4_GPIO_Port;

	//teraz ustawiam domyślne tryby silnika i sposoby poruszania
	FL->MOTOR_STATE=DRIVE;                             //domyślny tryb silnika to jazda-->raczej tutaj nic nie zmienimy
	FL->MOVE_STATE=FORWARD;                            //domyślny tryb to jazda do przodu

	FR->MOTOR_STATE=DRIVE;
	FR->MOVE_STATE=FORWARD;

	BL->MOTOR_STATE=DRIVE;
	BL->MOVE_STATE=FORWARD;

	BR->MOTOR_STATE=DRIVE;
	BR->MOVE_STATE=FORWARD;

	FL->ramp_data.RisingTime=0.0;
	FR->ramp_data.RisingTime=0.0;
	BL->ramp_data.RisingTime=0.0;
	BR->ramp_data.RisingTime=0.0;

	//teraz startuje timery
	HAL_TIM_PWM_Start(FL->timer_data._htim_PWM, FL->timer_data._channel_PWM);
	HAL_TIM_PWM_Start(FR->timer_data._htim_PWM, FR->timer_data._channel_PWM);
	HAL_TIM_PWM_Start(BL->timer_data._htim_PWM, BL->timer_data._channel_PWM);
	HAL_TIM_PWM_Start(BR->timer_data._htim_PWM, BR->timer_data._channel_PWM);
}


void PositionIncrement(WHEEL_Type* key)
{
	key->encoder_data.Value++; //inkrementuję wartość impulsów
}
void VelocityCalculation(WHEEL_Type* FL,WHEEL_Type* FR,WHEEL_Type* BL,WHEEL_Type* BR,int speed)
{                                          //ta funkcja oblicza jaką predkosc ma mieć kazde kolo
	if((FR->MOTOR_STATE==DRIVE)&&(FL->MOTOR_STATE==DRIVE)&&(BR->MOTOR_STATE==DRIVE)&&(BL->MOTOR_STATE==DRIVE))
	{                                                           //sprawdzam czy kazde kolo jest w stanie drive
		if((FR->MOVE_STATE==FL->MOVE_STATE)&&(BR->MOVE_STATE==BL->MOVE_STATE)&&(FL->MOVE_STATE==BR->MOVE_STATE))
		{                                               //sprawdzam czy kazde kolo jest w tym samym trybie ruchu
			switch(FR->MOVE_STATE)                    //a teraz sprawdzam w którym dokładnie trybie ruchu jestem
			{
			case IDLE_2:
				FL->ramp_data.FinalSpeed=0;      //w trybie nieustalonym predkosc wynosi 0
				FR->ramp_data.FinalSpeed=0;
				BL->ramp_data.FinalSpeed=0;
				BR->ramp_data.FinalSpeed=0;

				FL->pid_data.Integral=0;
				FR->pid_data.Integral=0;
				BL->pid_data.Integral=0;
				BR->pid_data.Integral=0;

				FL->pid_data.PidSignal=0;
				FR->pid_data.PidSignal=0;
				BL->pid_data.PidSignal=0;
				BR->pid_data.PidSignal=0;

				FL->motor_siganl=0;
				FR->motor_siganl=0;
				BL->motor_siganl=0;
				BR->motor_siganl=0;
				break;
			case FORWARD:
				FL->ramp_data.FinalSpeed=-speed;//w trybie do przodu - wszystkie koła krecą się w tą samą stronę
				FR->ramp_data.FinalSpeed=speed;
				BL->ramp_data.FinalSpeed=-speed;
				BR->ramp_data.FinalSpeed=speed;
				break;
			case RIGHT_SIDE:
				FL->ramp_data.FinalSpeed=-speed;
				FR->ramp_data.FinalSpeed=-speed;
				BL->ramp_data.FinalSpeed=speed;
				BR->ramp_data.FinalSpeed=speed;
				break;
			case AROUND:
				FL->ramp_data.FinalSpeed=-speed;
				FR->ramp_data.FinalSpeed=-speed;
				BL->ramp_data.FinalSpeed=-speed;
				BR->ramp_data.FinalSpeed=-speed;
				break;
			}

		}
	}
}

void EncoderVelocity(WHEEL_Type* key)               //tworzę funkcje która oblicza aktualną prędkość
{
	if(key->motor_siganl<0)    //sprawdzam czy silnik obraca się w lewą stronę czy prawą, od tego zależy znak predkosci
	{
		key->encoder_data.Value=key->encoder_data.Value*-1;
	}
	key->encoder_data.TotalTurns+=key->encoder_data.Value;        //uaktualniam liczbę wszytkich obrotów które zrobił silnik
	float alpha;
	if(key->encoder_data.Value!=0)
	{
		alpha = 0.01*(abs(key->encoder_data.Value));      //to zrobienie dynamicznego filtra - poprzez dynamiczną zmianę współczynnika wygładzającego
		key->encoder_data.CurrentVelocity=key->encoder_data.Value*alpha+(1-alpha)*key->encoder_data.V1;//Obliczam prędkosć z filtra ekspotencjalnego
		key->encoder_data.V1=key->encoder_data.CurrentVelocity;                    //cofam próbki o jeden
	}
	else
	{
		key->encoder_data.V1=0;
		key->encoder_data.CurrentVelocity=0;
	}
	key->encoder_data.Value=0;                                    //zeruję wartość imulsów
}

void MotorState(WHEEL_Type *key)
{
	switch(key->MOTOR_STATE)                    //a teraz sprawdzam w którym dokładnie trybie ruchu jestem
	{
		case IDLE_1: //w tym stanie predkosc wynosi 0
		 	__HAL_TIM_SET_COMPARE(key->timer_data._htim_PWM,key->timer_data._channel_PWM,0);    //ustawiam sygnał PWM 0%
		 	break; //zapobiega wykonywaniu kolejnych przypadków.
		 case DRIVE: //w tym stanie nie hamujemy i mamy zezwolnie na prace i ustawaim zadaną prędkość
		 	if(key->motor_siganl<0)              //gdy predkosc jest mniejsza od zera to obracam silnk w lewo a gdy wieksza to w prawo
		 	{
			  	HAL_GPIO_WritePin(key->pinout_data.DirectionPort,key->pinout_data.DirectionPin, RESET);      //gdy set to obroty w lewo
		 		__HAL_TIM_SET_COMPARE(key->timer_data._htim_PWM,key->timer_data._channel_PWM,abs(key->motor_siganl));
		 	}                                             //ustawiam żądany sygnał na odpowiednim timerze i kanale ale sygnał bez minusa
		 	else if(key->motor_siganl>=0)
		 	{
		 		HAL_GPIO_WritePin(key->pinout_data.DirectionPort,key->pinout_data.DirectionPin, SET);      //gdy reset to obroty w prawo
		 		__HAL_TIM_SET_COMPARE(key->timer_data._htim_PWM,key->timer_data._channel_PWM,key->motor_siganl);
		 	}                                                                    //ustawiam żądany sygnał na odpowiednim timerze i kanale
		 	break;
		 case BRAKE: //w tym stanie predkosc wynosi 0
			__HAL_TIM_SET_COMPARE(key->timer_data._htim_PWM,key->timer_data._channel_PWM,0);        //ustawiam sygnał PWM 0%
		 	break;
	}
}

void PID(WHEEL_Type* key)
{
	float P=0.267;     //definiuje współczynnik P,I,D
	float I=14.1;
	float D=0.0;
	key->pid_data.Deviation=key->set_velocity-key->encoder_data.CurrentVelocity;    //obliczam uchyb regulacji
	if(key->pid_data.PidSignal>-1000&&key->pid_data.PidSignal<1000)     //to zabezpieczenie przed windup
	{
		key->pid_data.Integral+=key->pid_data.Deviation*0.2;//obliczam całkę z uchybu 0.2 ponieważ bede tą funkcję wykonywał co 0.2s
	}
	else
	{
		if(key->encoder_data.CurrentVelocity==0)
		{
			key->pid_data.Integral=0;
		}
		if(key->encoder_data.CurrentVelocity!=0)
		{
			key->pid_data.Integral=key->pid_data.Integral;
		}
	}

	key->pid_data.PidSignal = key->pid_data.Deviation*P + key->pid_data.Integral*I + key->pid_data.Derivative*D;//to standardowy regulator PID
	key->motor_siganl=key->pid_data.PidSignal;
}

void SpeedRamp(WHEEL_Type *key)
{
	if(key->ramp_data.LastFinalSpeed!=key->ramp_data.FinalSpeed)
	{
		key->ramp_data.LastFinalSpeed=key->ramp_data.FinalSpeed;     //przypisuje starej predkosci nową prędkość
		key->ramp_data.StartSpeed=key->encoder_data.CurrentVelocity; //predkosc startująca to aktualna predkość
	}
	if(key->ramp_data.LastFinalSpeed==key->ramp_data.FinalSpeed)
	{
		key->ramp_data.Delta=(key->ramp_data.FinalSpeed-key->ramp_data.StartSpeed)/(key->ramp_data.RisingTime*5);
		//Obliczam o ile mam zwiększać predkosc z kolejnym krokiem + plus kontrola przed złym znakiem
		if(key->ramp_data.Delta<0)//dla rampy mniejszej od zera czyli zmniejszanie predkosci są inne warunki w if poniżej
		{
			if((key->set_velocity<(key->ramp_data.FinalSpeed-key->ramp_data.Delta))&&(key->set_velocity>(key->ramp_data.FinalSpeed+key->ramp_data.Delta)))
			{
				key->set_velocity=key->ramp_data.FinalSpeed;
			}
			else
			{
				key->set_velocity+=key->ramp_data.Delta;
			}
		}
		else if(key->ramp_data.Delta>0)
		{
			if((key->set_velocity>(key->ramp_data.FinalSpeed-key->ramp_data.Delta))&&(key->set_velocity<(key->ramp_data.FinalSpeed+key->ramp_data.Delta)))
			{
				key->set_velocity=key->ramp_data.FinalSpeed;
			}
			else
			{
				key->set_velocity+=key->ramp_data.Delta;
			}
		}
	}
}
int ReadInfoFromUart(char znak,WHEEL_Type* FL,WHEEL_Type* FR,WHEEL_Type* BL,WHEEL_Type* BR)
{
	int speed;
	switch(znak)                    //a teraz sprawdzam w którym dokładnie trybie ruchu jestem
	{
		case '0':
			FL->MOVE_STATE=IDLE_2;
			FR->MOVE_STATE=IDLE_2;
			BL->MOVE_STATE=IDLE_2;
			BR->MOVE_STATE=IDLE_2;
			speed=0;
			break;
		case 'a':                     //gdy przyjdzie 0 to jazda do przodu
			FL->MOVE_STATE=FORWARD;
			FR->MOVE_STATE=FORWARD;
			BL->MOVE_STATE=FORWARD;
			BR->MOVE_STATE=FORWARD;
			speed=12;
			break;
		case 'b':
			FL->MOVE_STATE=FORWARD;
			FR->MOVE_STATE=FORWARD;
			BL->MOVE_STATE=FORWARD;
			BR->MOVE_STATE=FORWARD;
			speed=-12;
			break;
		case 'c':
			FL->MOVE_STATE=AROUND;
			FR->MOVE_STATE=AROUND;
			BL->MOVE_STATE=AROUND;
			BR->MOVE_STATE=AROUND;
			speed=-12;
			break;
		case 'd':                    //gdy przyjdzie 3 to jazda w prawo
			FL->MOVE_STATE=AROUND;
			FR->MOVE_STATE=AROUND;
			BL->MOVE_STATE=AROUND;
			BR->MOVE_STATE=AROUND;
			speed=12;
			break;
		case 'e':
			FL->MOVE_STATE=RIGHT_SIDE;
			FR->MOVE_STATE=RIGHT_SIDE;
			BL->MOVE_STATE=RIGHT_SIDE;
			BR->MOVE_STATE=RIGHT_SIDE;
			speed=12;
			break;
		case 'f':
			FL->MOVE_STATE=RIGHT_SIDE;
			FR->MOVE_STATE=RIGHT_SIDE;
			BL->MOVE_STATE=RIGHT_SIDE;
			BR->MOVE_STATE=RIGHT_SIDE;
			speed=-12;
			break;
		case 'A':                     //gdy przyjdzie 0 to jazda do przodu
			FL->MOVE_STATE=FORWARD;
			FR->MOVE_STATE=FORWARD;
			BL->MOVE_STATE=FORWARD;
			BR->MOVE_STATE=FORWARD;
			speed=25;
			break;
		case 'B':
			FL->MOVE_STATE=FORWARD;
			FR->MOVE_STATE=FORWARD;
			BL->MOVE_STATE=FORWARD;
			BR->MOVE_STATE=FORWARD;
			speed=-25;
			break;
		case 'C':
			FL->MOVE_STATE=AROUND;
			FR->MOVE_STATE=AROUND;
			BL->MOVE_STATE=AROUND;
			BR->MOVE_STATE=AROUND;
			speed=-15;
			break;
		case 'D':                    //gdy przyjdzie 3 to jazda w prawo
			FL->MOVE_STATE=AROUND;
			FR->MOVE_STATE=AROUND;
			BL->MOVE_STATE=AROUND;
			BR->MOVE_STATE=AROUND;
			speed=15;
			break;
		case 'E':
			FL->MOVE_STATE=RIGHT_SIDE;
			FR->MOVE_STATE=RIGHT_SIDE;
			BL->MOVE_STATE=RIGHT_SIDE;
			BR->MOVE_STATE=RIGHT_SIDE;
			speed=25;
			break;
		case 'F':
			FL->MOVE_STATE=RIGHT_SIDE;
			FR->MOVE_STATE=RIGHT_SIDE;
			BL->MOVE_STATE=RIGHT_SIDE;
			BR->MOVE_STATE=RIGHT_SIDE;
			speed=-25;
			break;
		default:
			FL->MOVE_STATE=IDLE_2;
			FR->MOVE_STATE=IDLE_2;
			BL->MOVE_STATE=IDLE_2;
			BR->MOVE_STATE=IDLE_2;
			speed=0;
		    break;
	}
	return speed;
}
#endif /* INC_BIB_H_ */
