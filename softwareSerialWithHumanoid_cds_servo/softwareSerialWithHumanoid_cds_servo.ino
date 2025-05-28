#include <SoftwareSerial.h>
#include <Servo.h>

// 초음파 센서 핀 정의
const int trigPin = 4;
const int echoPin = 5;

bool bStart = false;
int nVol, cdsVol, btnVol;
bool bRobotStanding = true;
float duration, distance;

#define LEDRED  9
#define LEDYEL  10
#define LEDGRN  11
#define VOLSEN  A0
#define CDSSEN  A1
#define BTN1    8

SoftwareSerial mySerial(2, 3); // RX, TX
Servo myservo;

int lastMotion = -1;
unsigned long lastButtonPress = 0;

void setup() {
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDYEL, OUTPUT);
  pinMode(LEDGRN, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(VOLSEN, INPUT);
  pinMode(CDSSEN, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  mySerial.begin(115200);
  Serial.println("프로그램 시작");

  myservo.attach(6); // 충돌 방지를 위해 서보모터는 6번 핀 사용
}

void robotCon(int nMotion) {
  if (lastMotion == nMotion) return; // 중복 동작 방지
  lastMotion = nMotion;
  unsigned char exeCmd[15] = {0xff, 0xff, 0x4c, 0x53,
                              0x00, 0x00, 0x00, 0x00,
                              0x30, 0x0c,
                              0x03,
                              0x01, 0x00, nMotion,
                              0x00};
  for (int i = 6; i < 14; i++) exeCmd[14] += exeCmd[i];
  mySerial.write(exeCmd, 15);
  delay(50);
}

void readSensor() {
  nVol = analogRead(VOLSEN);
  cdsVol = analogRead(CDSSEN);
  btnVol = digitalRead(BTN1);
}

void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * .0343) / 2;
}

bool checkButtonToggle() {
  if (btnVol && millis() - lastButtonPress > 500) {
    lastButtonPress = millis();
    return true;
  }
  return false;
}

void ledRobotStat() {
  digitalWrite(LEDGRN, bRobotStanding);
  digitalWrite(LEDRED, !bRobotStanding);
}

void loop() {
  readSensor();
  readDistance();

  if (checkButtonToggle()) {
    bStart = !bStart;
  }

  if (bStart) {
    robotCon(19); // 인사 동작
    delay(3000);
    bStart = false;
  }

  bool isBright = cdsVol > 500;

  if (!isBright || distance > 150) {
    if (bRobotStanding) {
      robotCon(115); // 앉기
      myservo.write(50);
      bRobotStanding = false;
    }
  } else if (distance >= 100 && distance <= 150) {
    if (!bRobotStanding) {
      robotCon(116); // 일어서기
      myservo.write(150);
      bRobotStanding = true;
    }
  } else if (distance >= 50 && distance < 100) {
    if (bRobotStanding) {
      robotCon(19); // 인사
    }
  } else if (distance < 50) {
    if (bRobotStanding) {
      robotCon(22); // 전투 태세
    }
  }

  ledRobotStat();
  delay(200); // 주기 제한
}