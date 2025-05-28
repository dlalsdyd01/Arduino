#include <SoftwareSerial.h> //라이브러리를 이용
#include <Servo.h>

// 초음파 센서 핀 정의
const int trigPin = 4;
const int echoPin = 5;

bool bStart = false; // 전역 변수로 뺀다.
int nVol, cdsVol, btnVol;
bool bRobotStanding = true; // 서 있으면 t 앉아 있으면 f으로 하기로 하자 아래는 선택적으로 활용
//int nRobotStanding = 1;     // 서 있으면 1 앉아 있으며 0등으로 할수도 있다. 이것은 다양한 동작일때 유효

float duration, distance; // 초음파 거리 센서용 변수

#define LEDRED  9
#define LEDYEL  10
#define LEDGRN  11
#define VOLSEN  A0
#define CDSSEN  A1
#define BTN1    8

SoftwareSerial mySerial(2, 3); // RX, TX // 소프트웨어 시리얼을 모자란 통신 포트를 활용하기 위함이다.
Servo myservo;  // create Servo object to control a servo

void setup() { // 프로그램을 작성하는데 필요한 가장 기본적인 셋팅을 하는 함수 이다. 1회만 실행 된다.
  // put your setup code here, to run once:
  // 핀의 용도를 정하자
  // 출력용도로 디지털핀 9, 10, 11을 LED에 할당
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDYEL, OUTPUT);
  pinMode(LEDGRN, OUTPUT);
  // 입력 용도로 디지털핀 8번을 버튼에 할당
  pinMode(BTN1, INPUT);
  // 입력용도로 아날로그 핀 A0을 가변저항에 할당
  pinMode(VOLSEN, INPUT);
  pinMode(CDSSEN, INPUT); // A1은 cds 센서를 사용할 예정이다.

  // 초음파 센서 핀 설정
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 아두이노의 디버깅을 위해서 씨리얼 포트를 이용하기 위해 활성화
  Serial.begin(115200); // 가장 일반적으로 사용하는 속도 9600bps를 지정
  Serial.println("------------ 프로그램 시작 -----------"); // 내용을 프린트 하고 다음 줄로 넘김(Line Feed)
  mySerial.begin(115200);
  Serial.println("--------software Serial 시작 --------");
  myservo.attach(9);  // attaches the servo on pin 9 to the Servo object
}

void ledCon(int inVol)
{
  digitalWrite(LEDGRN, LOW);
  digitalWrite(LEDRED, HIGH);
  delay(inVol);               // 너무 자주 읽지 않도록 Delay 0.1초
  digitalWrite(LEDRED, LOW);
  digitalWrite(LEDYEL, HIGH);
  delay(inVol);               // 너무 자주 읽지 않도록 Delay 0.1초
  digitalWrite(LEDYEL, LOW);
  digitalWrite(LEDGRN, HIGH);
  delay(inVol);               // 너무 자주 읽지 않도록 Delay 0.1초
}

void robotCon(int nMotion)
{
  // 휴머노이드 로봇을 제어 하는 방
  // 아래 exeCmd는 python에서 로봇을 컨트롤 하던 프로토콜과 동일
  unsigned char exeCmd[15] = {0xff, 0xff, 0x4c, 0x53, // header
                              0x00, 0x00, 0x00, 0x00, // address 2개
                              0x30, 0x0c,             // 30 0c는 모션실행
                              0x03,                   // 파라메타의 길이
                              0x01, 0x00, 100,         // 1번동작 반복 속도
                              0x00};                  // checkSum
  exeCmd[11] = nMotion;   // 실제 동작할 모션 번호를 입력
  exeCmd[14] = 0x00;      // checksum을 클리어
  for (int i = 6; i < 14; i++) // checksum을 계산
    exeCmd[14] += exeCmd[i];  // Checksum을 입력

  mySerial.write(exeCmd, 15);   // 버퍼의 내용을 쓰기
  delay(50);                    // 명령이 전달되는 최소 시간
}

void readSensor()
{
  nVol = analogRead(VOLSEN);    // cVol, nVol에 아나로그 A0의 값을 읽어서 대입
  cdsVol = analogRead(CDSSEN);
  btnVol = digitalRead(BTN1);

  Serial.print("btnVol = "); //ln이 없는 print문
  Serial.print(btnVol);
  Serial.print("\t cdsVol = "); //ln이 없는 print문
  Serial.print(cdsVol);
  Serial.print("\t nVol(가변저항) = ");
  Serial.print(nVol);     // 시리얼포트에 쓰기
  Serial.print("\t Distance = "); // 추가: 거리 값 출력
  Serial.println(distance);
}

// 초음파 센서 거리 측정 함수
void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * .0343) / 2;
}

bool toggleCheck()
{
  if (btnVol) return true;
  return false;
}

void ledRobotStat()
{
  digitalWrite(LEDGRN, bRobotStanding); //true는 1이고 켜는거고 high이고 참이다.
}

void loop()
{
  readSensor();
  readDistance(); // 거리 센서 값 읽기

  if (toggleCheck()) {
    bStart = !bStart;
    delay(200); // 버튼 디바운싱을 위한 딜레이 추가
  }

  // 버튼을 눌렀을 때 19번 동작 수행 (기존 기능 유지)
  if (bStart)
  {
    robotCon(19);
    delay(7000);    //로봇이 동작을 완료 할때까지 기다리는 시간
    bStart = false; // 한 번 동작 후 bStart를 false로 초기화
  }
  else
  {
    //Serial.println("Waiting button"); // 디버깅 메시지, 필요 시 활성화
  }

  // CDS 센서와 초음파 센서 값을 기반으로 로봇 동작 제어
  if (cdsVol > 500) { // 밝은 경우 (CDS 센서 조건)
    if (distance > 150) { // 150cm 이내에 아무도 없거나 밝으면 앉아서 기다림
      if (bRobotStanding) { // 현재 서있으면 앉기 동작 수행
        robotCon(115); // 앉기 동작
        delay(7000);
        bRobotStanding = false;
        myservo.write(50);
      } else { // 이미 앉아 있으면 LED 상태 업데이트
        ledRobotStat();
      }
    } else if (distance >= 100 && distance <= 150) { // 100~150cm 내에 사람이 나타나면 일어서서 대기
      if (!bRobotStanding) { // 현재 앉아 있으면 일어서기 동작 수행
        robotCon(116); // 일어서기 동작
        delay(7000);
        bRobotStanding = true;
        myservo.write(150);
      } else { // 이미 서있으면 LED 상태 업데이트
        ledRobotStat();
      }
    } else if (distance >= 50 && distance < 100) { // 100~50cm 이내로 접근하면 인사
      if (bRobotStanding) { // 서 있는 상태에서만 인사
        robotCon(19); // 인사 동작
        delay(7000);
      }
    } else if (distance < 50) { // 50cm 이내로 접근하면 전투 태세 (동작 번호 확인 필요)
      if (bRobotStanding) { // 서 있는 상태에서만 전투 태세
        robotCon(22); // 예시: 전투 태세 동작 번호 (실제 번호로 변경 필요)
        delay(7000);
      }
    }
  } else { // 어두운 경우 (CDS 센서 조건)
    if (bRobotStanding) { // 어두우면 앉아서 기다림
      robotCon(115); // 앉기 동작
      delay(7000);
      bRobotStanding = false;
      myservo.write(50);
    } else { // 이미 앉아 있으면 LED 상태 업데이트
      ledRobotStat();
    }
  }
}