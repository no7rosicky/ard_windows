
#include <SoftwareSerial.h> //아두이노 시리얼통신을 위한 헤더파일
#include "MQ7.h" //일산화탄소 센서 헤더파일
#include <pm2008_i2c.h> //미세먼지 센서 헤더파일
#include <Stepper.h> // 스텝모터 헤더파일

const int stepsPerRevolution = 256; //모터 회전 각도 설정
Stepper myStepper(stepsPerRevolution,11,9,10,8); // 모터 인스턴스 생성, 8~11번 핀 연결
#define rain 2 //rain 이라는 이름은 앞으로 2로 정의
int bz = 4; //4번포트는 일산화탄소 부저
MQ7 mq7(A5, 5.0); // 일산화탄소 센서 인스턴스 생성, 5번핀과 연결
PM2008_I2C pm2008_i2c; // 미세먼지 센서 인스턴스 생성
int motor_flag = 0;
int co_flag = 0; // 정수 자료형으로 각각 센서에서 모터를 동작 시킬지 말지를 0과 1로 결정한다
int rain_flag = 0;
int dust_flag = 0;

void open_window(int cycle)
{
  Serial.println("Open");
  myStepper.step(-stepsPerRevolution * cycle); //스텝모터를 구동시키는 코드, 256* cycle만큼 돌린다 void loop()에 open_window(8)이므로 256 * 8 = 2048 (시계방향 1바퀴)
  motor_flag = 0;
}

void close_window(int cycle)
{
  Serial.println("Close");
  myStepper.step(stepsPerRevolution * cycle);
  motor_flag = 1;
}

void setup() { // 아두이노 시작시에만 실행되는곳
  Serial.begin(9600); // 보드와 통신속도 설정
  myStepper.setSpeed(40); //모터 속도 설정
  pm2008_i2c.begin(); // 미세먼지센서 통신설정
  pinMode(rain, OUTPUT); //2번핀 출력설정 
  pinMode(bz, OUTPUT); //4번핀 출력설정
  pm2008_i2c.command(); //미세먼지센서 명령 설정
  Serial.println("Arduino Starting...");
  open_window(10);
  co_flag = 0;
  rain_flag = 0;
  dust_flag = 0;
  delay(10);
}

int co_measure(){ //일산화탄소 센서에서 
  Serial.println(" * CO Sensor * ");

  float filteredValue = 0; // 부동소수점 자료형 filterdValue를 선언, 
  for(int i = 0; i < 100; i++) // 100번동안 측정하는 과정
  {
    filteredValue += mq7.getPPM(); 
    delayMicroseconds(100);
  }
  filteredValue /= 100; //100번의 평균값을 도출
  Serial.print("CO: "); //시리얼 통신(노트북 출력)으로 프린트할 내용
  Serial.println(filteredValue); //100번 측정한 평균값 출력
  Serial.print("ppm");
  Serial.println("");
  
  if(50 <= filteredValue)
  { //if 조건문 측정한 센서의 평균값이
    digitalWrite(bz,HIGH); //50 이상이라면 부저를 울림
    delay(10);
    return 1;
  }
  else
  {
    digitalWrite(bz,LOW); //50미만이면 부저 끔
    delay(10);
    return 0;
  }
}

int rain_measure(){ // 빗물감지센서에서 데이터를 받아 빗물이 있다면 1을반환, 없다면 0을 반환
  Serial.println(" * Rain Sensor * ");

  if(digitalRead(rain) == HIGH) //2번핀에 연결된 센서에 물이 감지되지 않았다면 
  {
    Serial.println("No Rain");
    delay(10);
    return 0;
  }
  else // 감지됬다면
  {
    Serial.println("Rain");
    delay(10);
    return 1;
  }
}

int dust_measure(){ //미세먼지 센서에서 데이터를 받아오는 함수
  Serial.println(" * Dust Sensor * ");
  uint8_t ret = pm2008_i2c.read(); //미세먼지 센서에서 값을 읽어와 ret라는 uint8_t 타입 변수에 저장
  if(ret == 0) 
  {
    Serial.print("PM 10 (GRIMM) : : ");
    Serial.println(pm2008_i2c.pm10_grimm);
    if(0 <= pm2008_i2c.pm10_grimm && pm2008_i2c.pm10_grimm < 80) // pm10기준 수치가 0 ~ 80
    {
      delay(10);
      return 0;
    } 
    else // 그이상일시
    {
      delay(10);
      return 1;
    }

  }
}

void loop() {
  Serial.println("Cycle begin------------------------");
  co_flag = co_measure(); // 위에 정의된 함수를 실행시켰을때 일산화탄소 농도가 일정 이상이면 1을 반환하여 co_flag 변수에 저장해둔다
  rain_flag = 0 // 얘내도 마찬가지
  dust_flag = dust_measure();

  if(co_flag == 1) //일산화탄소가 50이상이라면 1이 반환되므로 if 조건문에 맞아 문을연다
  {
    if(motor_flag == 1) // 창문이 닫혀있다면
    {
      open_window(10);
      
    } 
  }
  else if(co_flag == 0 && rain_flag == 0 && dust_flag == 0)  //그외의 상황은 모두 문을 연다
  {
    if(motor_flag == 1)
    {
      open_window(10);
    }
  }
  else if(co_flag == 0 && (rain_flag == 1 || dust_flag == 1)) //50이하면서 빗물이나 미세먼지 둘중하나가 감지된다면 문을 닫느다 
  {
    if(motor_flag == 0) // 창문이 열려있다면
    {
      close_window(10);

    }
  }  
  Serial.println("Cycle end------------------------");
  Serial.println("");
}
