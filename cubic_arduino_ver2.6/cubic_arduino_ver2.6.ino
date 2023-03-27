#include "cubic_arduino_ver2.6.h"

/*モーターの利用*/
// put関数で各モータのduty比指定，duty比の符号反転で逆回転
// Dutyの最大値はデフォルトで1000

/*エンコーダの利用*/
// get関数で各エンコーダの値を取得．Inc_encは1ms辺りの変化量，Abs_encは絶対位置[0, 16383]

/*ソレノイドの利用*/
// put関数で各ソレノイドの状態指定
// get関数で各ソレノイドの状態を取得

// 各種print関数はSerial.begin()を実行したあとでないと使えない．(ROSと一緒には使えない)

// 詳しくはヘッダファイル参照

void setup() {
  // すべてのモータ，エンコーダの初期化
  Cubic::begin();
  Serial.begin(115200);
}

int duty = 0;
//int16_t duty[] = {-120, -153, -130};
//int16_t duty[] = {220, 200, 150};
bool state = 1;

void loop() {
  // シリアル入力でdutyまたはstateの値を指定
  if (Serial.available() > 0) {
    ///*
    uint8_t i = Serial.readStringUntil(':').toInt();
    duty = Serial.readStringUntil('\n').toInt();
    DC_motor::put(i, duty);
    //*/
    //state = Serial.readStringUntil('\n').toInt();
    /*
    String s = Serial.readStringUntil('\n');
    if (s == "r") {
      for (int i = 0; i < 3; i++) duty[i] *= -1;
      DC_motor::put(0, duty[0]); DC_motor::put(2, duty[1]); DC_motor::put(6, duty[2]);
    }    
    else {
      DC_motor::put(0, 0); DC_motor::put(2, 0); DC_motor::put(6, 0);
    }
    */
  }

  for (int i = 0; i < 8; i++) {
    // i番モータにdutyを指定
    //DC_motor::put(i, duty);

    // i番インクリメントエンコーダの値の差分を取得し表示
    int enc_diff = Inc_enc::get(i);
//    Serial.print(enc_diff); Serial.print(" ");
  }
  //Serial.print(" ");

  // すべてのアブソリュートエンコーダの値を表示
//  Abs_enc::print();

  for (int i = 0; i < 4; i++) {
    // 各ソレノイドの状態を格納
    Solenoid::put(i, state);
  }
  // すべてのソレノイドの状態を表示
//  Solenoid::print();

  // すべてのモータのDuty(最大32766)を表示
//  DC_motor::print(true);

  // データの送受信を行う
  Cubic::update();

  delay(1);
}
