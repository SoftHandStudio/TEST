/*
 * NG版
 * 
*/

#include <SimpleSDAudio.h>
#include <SoftwareSerial.h>

/*----------定義模組連接 PIN 腳位-----------*/
#define RELAY_1 2   // 繼電器_1
#define RELAY_2 3   // 繼電器_2
#define HandSet 6   // 電話控制按鈕
#define BTTX 7      // 藍芽模組 TX
#define BTRX 8      // 藍芽模組 RX
/*
    SD卡接線: MOSI 11, SCK 13, MISO 12, CS 4
    音訊輸出(LM386) 9
*/

SoftwareSerial BT(BTTX, BTRX);           // 藍芽模組 (TX , RX) Pin

/*--------變數宣告--------*/
char val, recieve;                     // 儲存藍芽接收資料的變數
int state = 0, age = 0, flag = 0;      // 儲存工作狀態的變數
char cmd, AudioFileName[6];            // 儲存檔名及指令的變數
boolean redo = false;                  // 儲存工作狀態(是否重新執行)
int relay1State = 0, relay2State = 1;  // 繼電器1狀態ON , 繼電器2狀態OFF

// Callback target, prints output to serial
void DirCallback(char *buf) {
  Serial.println(buf);
}

// Create static buffer
#define BIGBUFSIZE (2*512)      // bigger than 2*512 is often only possible on Arduino megas!
uint8_t bigbuf[BIGBUFSIZE];

// helper function to determine free ram at runtime
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

/*-------------隨機故事檔案名稱-------------*/
char RingTone[6] = "RN.AFM";
String NG = "NG.AFM", strFName = "";
String storyfile[][6] = {{"A1.AFM", "A2.AFM", "A3.AFM", "NG.AFM", "NG.AFM", "NG.AFM"}, {"B1.AFM", "B2.AFM", "B3.AFM", "NG.AFM", "NG.AFM", "NG.AFM"}};
String randomfile(int age) {
  return storyfile[age][random(6)];
}

/*---------------搜尋檔案名稱----------------*/
void findFile(char fName[6]) {
  for (int i = 0; i < 6; i++) {
    AudioFileName[i] = fName[i];
  }
  Serial.println(AudioFileName);
}

void findStrFile(String str) {
  char char_array[7];
  str.toCharArray(char_array, 7);
  for (int i = 0; i < 6; i++) {
    AudioFileName[i] = char_array[i];
  }
  Serial.println(AudioFileName);
}

void searchFileFromSD() {
  Serial.print(F("Looking for file... "));
  if (!SdPlay.setFile(AudioFileName)) {
    Serial.println(F(" not found on card! Error code: "));
    Serial.println(SdPlay.getLastError());
  } else {
    Serial.println(F("found."));
  }
}

/*----------------繼電器控制---------------*/
void switchOnRelay1() {
  relay1State = 0;                       // 把繼電器1狀態改為 ON
  digitalWrite(RELAY_1, relay1State);  // 讓繼電器作動, 切換開關
}

void switchOffRelay1() {
  relay1State = 1;                       // 把繼電器1狀態改為 OFF
  digitalWrite(RELAY_1, relay1State);  // 讓繼電器作動, 切換開關
}

void switchOnRelay2() {
  relay2State = 0;                       // 把繼電器2狀態改為 ON
  digitalWrite(RELAY_2, relay2State);  // 讓繼電器作動, 切換開關
}

void switchOffRelay2() {
  relay2State = 1;                       // 把繼電器2狀態改為 OFF
  digitalWrite(RELAY_2, relay2State);  // 讓繼電器作動, 切換開關
}

/*---------------------------setup()------------------------------*/
void setup() {
  // Open serial and BlueTooth communications and wait for port to open:
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(RELAY_1, OUTPUT);        // 把 RELAY_1 設置成 OUTPUT
  pinMode(RELAY_2, OUTPUT);        // 把 RELAY_2 設置成 OUTPUT
  pinMode(HandSet, INPUT_PULLUP);  // 把 電話控制鈕 設置成 INPUT_PULLUP

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  /*----偵測 SD記憶卡 模組是否連接正確----*/
  Serial.print(F("Free Ram: "));
  Serial.println(freeRam());

  // Setting the buffer manually for more flexibility
  SdPlay.setWorkBuffer(bigbuf, BIGBUFSIZE);

  Serial.print(F("\nInitializing SimpleSDAudio V" SSDA_VERSIONSTRING " ..."));

  // If your SD card CS-Pin is not at Pin 4, enable and adapt the following line:
  // SdPlay.setSDCSPin(10);

  // Select between SSDA_MODE_FULLRATE or SSDA_MODE_HALFRATE (62.5kHz or 31.25kHz)
  // and the output modes SSDA_MODE_MONO_BRIDGE, SSDA_MODE_MONO or SSDA_MODE_STEREO
  if (!SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_MONO)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card is inserted?"));
    Serial.println(F("* Is your wiring correct?"));
    Serial.println(F("* maybe you need to change the chipSelect pin to match your shield or module?"));
    Serial.print(F("Error code: "));
    Serial.println(SdPlay.getLastError());
    while (1);
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }
}

/*---------------------------loop(()------------------------------*/
void loop() {
  if (BT.available()) {
    val = BT.read();
    Serial.println(val);
  }

  digitalWrite(RELAY_1, relay1State);
  digitalWrite(RELAY_2, relay2State);

  switch (val) {
    case 'A':
    case 'a':
      age = 0;
      state = 1;
      break;
    case 'B':
    case 'b':
      age = 1;
      state = 1;
      break;
    //    case 'C':
    //    case 'c':
    //      age = 2;
    //      state = 1;
    //      break;
    //    case 'D':
    //    case 'd':
    //      age = 3;
    //      state = 1;
    //      break;
    //    case 'E':
    //    case 'e':
    //      age = 4;
    //      state = 1;
    //      break;
    //    case 'F':
    //    case 'f':
    //      age = 5;
    //      state = 1;
    //      break;
    //    case 'G':
    //    case 'g':
    //      age = 6;
    //      state = 1;
    //      break;
    default:
      break;
  }

  switch (state) {
    case 1:
      Serial.println(F("------state1------"));
      // 隨機延遲小於2秒，使電話能此起彼落響起
      int delayTime;
      delayTime = random(2000);
      delay(delayTime);
      Serial.print(F("delay: "));
      Serial.print(delayTime);
      Serial.println(F(" ms"));
      // 切換繼電器開關
      switchOffRelay1();
      switchOnRelay2();
      // 搜尋檔案名稱
      findFile(RingTone);
      searchFileFromSD();
      cmd = 'p';
      while (digitalRead(HandSet)) {
        recieve =  BT.read();
        SdPlay.worker();
        switch (cmd) {
          case 'p':
            SdPlay.play();
            Serial.println(F("Play."));
            break;
          default:
            break;
        }
        if (recieve == 's') {
          Serial.println(F("Stop."));
          Serial.println(F("------end------"));
          break;
        }
        if (SdPlay.isStopped()) {
          Serial.println(F("Stop."));
          cmd = 'p';
        } else {
          cmd = ' ';
        }
      }
      if (recieve == 's') {
        state = 0;
      } else {
        state++;
      }
      break;
    case 2:
      Serial.println(F("------state2------"));
      // 切換繼電器開關
      switchOnRelay1();
      switchOffRelay2();
      // 搜尋隨機檔案名稱
      strFName = randomfile(age);
      findStrFile(strFName);
      searchFileFromSD();
      cmd = 'p';
      flag = 1;
      while (flag != digitalRead(HandSet)) {
        SdPlay.worker();
        switch (cmd) {
          case 'p':
            SdPlay.play();
            Serial.println(F("Play."));
            BT.print('t');
            break;
          default:
            break;
        }
        if (SdPlay.isStopped()) {
          Serial.println(F("Stop."));
          flag = 0;
        } else {
          cmd = ' ';
        }
      }

      if (digitalRead(HandSet) == 0) {
        if (strFName == NG) { // 若隨機檔案名稱為 NG 則播放完後重新執行
          Serial.println(F("------redo-----"));
          while (!digitalRead(HandSet));
          BT.print('r');
        } else {
          Serial.println(F("------end------"));
          BT.print('f');
        }
      } else if (digitalRead(HandSet) == 1) {
        if (strFName == NG) { // 若隨機檔案名稱為 NG 則播放完後重新執行
          Serial.println(F("------redo-----"));
          BT.print('r');
        } else {
          Serial.println(F("------end------"));
          BT.print('f');
        }
      }
      state = 0;
      break;
    default:
      break;
  }
  val = ' ';
  recieve = ' ';
}
