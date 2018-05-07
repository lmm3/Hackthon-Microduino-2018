#include <Arduino.h>
#include <Microduino_Key.h>
#include <IRremote.h>
#include <Microduino_ColorLED.h>
#include <Microduino_Tem_Hum.h> //biblioteca para temperatura
#include <U8glib.h> //biblioteca LCD
#include <Microduino_Motor.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
#define TEMP "IIC"  //definição da pinagem
#define sensorPin  A0

Tem_D1  termo;

VirtualKey keyVirtual;
IRrecv irrecv(4);
decode_results results;

Motor MotorLeft(MOTOR0_PINA, MOTOR0_PINB);
Motor MotorRight(MOTOR1_PINA, MOTOR1_PINB);

uint32_t irData = 0x00000000;
uint32_t irDataCache = 0x00000000;
uint32_t _irData = 0x00000000;
uint32_t _irDataCache = 0x00000000;
boolean buttonsta[3];
int state;
float newState;

uint32_t dump(decode_results *results)
{
    int count = results->rawlen;
    if (results->decode_type == UNKNOWN)
    {
    }
    else
    {
        if (results->decode_type == NEC)
        {
        }
        _irDataCache = _irData;
        if (results->value == 0xFFFFFFFF)
        {
            _irData = _irDataCache;
        }
        else
        {
            _irData = results->value;
        }
    }
}

uint32_t _irTime = 0;
uint32_t irFluse()
{
    if (irrecv.decode(&results))
    {
        dump(&results);
        irrecv.resume();
        _irTime = millis();
    }
    if (millis() - _irTime > 200)
    {
        _irData = 0x00000000;
    }
    return _irData;
}

void irDataShift()
{
    irData = irFluse();
    switch (keyVirtual.readVal(irData))
    {
        case KEY_RELEASED:
        buttonsta[0] = false;
        break;
        case KEY_PRESSED:
        irDataCache = irData;
        buttonsta[0] = true;
        break;
        case KEY_PRESSING:
        buttonsta[1] = true;
        break;
        case KEY_RELEASING:
        buttonsta[2] = true;
        break;
    }
}

bool irButton(uint8_t sta, uint32_t _irButton)
{
    irDataShift();
    if (!buttonsta[0] && (sta == 1) && (irData == 0))
    return true;
    else if (buttonsta[0] && (sta == 2) && (irData == _irButton))
    return true;
    else if (buttonsta[1] && (sta == 0) && (irData == _irButton))
    {
        buttonsta[1] = false;
        return true;
    }
    else if (buttonsta[2] && (sta == 3) && (irData == 0) && (irDataCache == _irButton))
    {
        buttonsta[2] = false;
        return true;
    }
    return false;
}

void read_IRcontrol();

ColorLED strip_2 = ColorLED(16, 2);
uint8_t estado;

void draw_termo(void) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_7x13);
  u8g.setPrintPos(0, 20); 
  // call procedure from base class, http://arduino.cc/en/Serial/Print
  u8g.print ("Temp.: ");
  u8g.print (termo.getTemperature());
  u8g.print ("ºC");
}

void draw_lumi(void)
{
  u8g.setFont(u8g_font_7x13);
  u8g.setPrintPos(0, 20);
  u8g.print("Lum: ");
  u8g.print(newState);
  u8g.print("%");
}



void setup()
{
    Serial.begin(9600);
    keyVirtual.begin();
    irrecv.enableIRIn();

    strip_2.begin();
    MotorLeft.begin();   //电机MotorLeft初始化
    MotorRight.begin();

}

void loop()
{
  switch(estado){
    case 1:
        //código exp1
        MotorLeft.Brake();        //电机MotorLeft刹车
        MotorRight.Brake();
        u8g.firstPage();
        do {
          draw_termo();
        }while (u8g.nextPage());
        //Serial.println(estado); debug code
        break;
    case 2:
        //código exp2
        MotorLeft.Brake();        //电机MotorLeft刹车
        MotorRight.Brake();
        state = analogRead(sensorPin);
        newState = (float) state/10.23;
        u8g.firstPage(); 
         do {
          draw_lumi();
        }while (u8g.nextPage());
        //Serial.println(estado); debug code
        break;
    case 3:
        //código exp3
        MotorLeft.setSpeed(FREE);  //设置电机MotorLeft为释放状态，即速度为0
        MotorRight.setSpeed(FREE);
        MotorLeft.setSpeed(30*8);   //设置电机MotorLeft速度为100
        MotorRight.setSpeed(25*4);
        //Serial.println(estado); debug code
        break;
    default:
        //Serial.println("ERRO"); debug code
        break;
  }
  
  read_IRcontrol();

}


void read_IRcontrol(){
  if(irButton(0,0x1FE807F))
    {
            strip_2.setPixelColor(0,0x2dff04);
            strip_2.show();
            estado = 1;
    }
    if(irButton(0,0x1FE40BF))
    { 
            strip_2.setPixelColor(0,0x2d04ff);
            strip_2.show();
            estado=2;
    }
    if( irButton(0,0x1FEC03F) )
    {
            strip_2.setPixelColor(0,0xff0404);
            strip_2.show();
            estado=3;
    }
}

