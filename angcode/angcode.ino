
#include <SoftwareSerial.h>
#include "MQ7.h" //이산화탄소 센서
#include <pm2008_i2c.h> //미세먼지 센서

#define rain 2 //빗물감지 센서
int bz = 4; //4번핀

PM2008_I2C pm2008_i2c;
//Mega 보드는 4개의 시리얼 연결을 지원 :
//Serial: 0 (RX) and 1 (TX); Serial 1: 19 (RX) and 18 (TX); Serial 2: 17 (RX) and 16 (TX); Serial 3: 15 (RX) and 14 (TX)

void setup() {

  pm2008_i2c.begin();
  Serial.begin(9600);
  pinMode(rain, OUTPUT);
  pinMode(bz, OUTPUT);
  pm2008_i2c.command();
  Serial.println("Arduino Starting");
  delay(500);

}

void co2_measure(){
  Serial.println("----------CO2----------");

  float filteredValue = 0;
  for(int i = 0; i < 100; i++)
  {
    filteredValue += mq7.getPPM();
    delayMicroseconds(100);
  }
  filteredValue /= 100;
  Serial.print("CO: "); //시리얼 통신으로 프린트할 내용
  Serial.println(filteredValue); //100번 측정한 평균값 출력
  Serial.println("ppm");
  if(50 <= filteredValue)
  { //if 조건문 측정한 센서의 평균값이
    digitalWrite(bz,HIGH); //50 이상이라면 부저를 울림
  }
  else
  {
    digitalWrite(bz,LOW); //50미만이면 부저 끔
  }
}

void humid_measure(){
  Serial.println("----------Rain----------");

  if(digitalRead(rain) == HIGH)
    Serial.println("No Rain");
  else
    Serial.println("Rain");

  delay(400);
}

void dust_measure(){
  Serial.println("----------Dust----------");
  uint8_t ret = pm2008_i2c.read();
  if (ret == 0) 
  {
    Serial.print("PM 1.0 (GRIMM) : ");
    Serial.println(pm2008_i2c.pm1p0_grimm);
    Serial.print("PM 2.5 (GRIMM) : : ");
    Serial.println(pm2008_i2c.pm2p5_grimm);
    Serial.print("PM 10 (GRIMM) : : ");
    Serial.println(pm2008_i2c.pm10_grimm);
  }

  delay(400);
}

void loop() {

  co2_measure();
  humid_measure();
  dust_measure();

  switch;// 각 함수들 return값 int형, 숫자에 따라 스위치문 작동
  
  
}
