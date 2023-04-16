#include "cubic_arduino.h"

/*モーターの利用*/
// put関数で各モータのduty比指定，duty比の符号反転で逆回転
// Dutyの最大値はデフォルトで1000

/*インクリメントエンコーダの利用*/
// get関数で各エンコーダの累積値を取得．get_diff関数で各エンコーダの差分値を取得．

/*アブソリュートエンコーダの利用*/
// get関数で各エンコーダの絶対位置[0, 16383]を取得．

/*ソレノイドの利用*/
// put関数で各ソレノイドの状態指定
// get関数で各ソレノイドの状態を取得

// 各種print関数はSerial.begin()を実行したあとでないと使えない．(ROSと一緒には使えない)

// 詳しくはヘッダファイル参照

long cnt = 0;
unsigned long time_prev, time_now;
float dt;

void setup() {
  // すべてのモータ，エンコーダの初期化
  Cubic::begin();
  Serial.begin(115200);

  time_prev = micros();
}

void loop() {
  // シリアル入力で動作モードを指定
  ///*
  if (Serial.available() > 0) {
    char mode = Serial.read();

    // モータのDutyを指定（例："m1:100"）
    if (mode == 'm') {
      uint8_t i = Serial.readStringUntil(':').toInt();
      int duty = Serial.readStringUntil('\n').toInt();
      DC_motor::put(i, duty);
    }
    // ソレノイドの状態を指定（例："s2:0"）
    else if (mode == 's') {
      uint8_t i = Serial.readStringUntil(':').toInt();
      bool state = Serial.readStringUntil('\n').toInt();
      Solenoid::put(i, state);
    }
    // インクリメントエンコーダの累積値をリセット
    else if (mode == 'r') {
      Inc_enc::reset();
    }
  }
  //*/

  // すべてのインクリメントエンコーダの累積値を表示
  Inc_enc::print();

  // すべてのインクリメントエンコーダの差分値を表示
  Inc_enc::print_diff();

  // すべてのアブソリュートエンコーダの値を表示
  Abs_enc::print();

  // すべてのDCモータのDutyを表示
  DC_motor::print();

  /*
  cnt++;
  if (cnt == 1000) {
    time_now = micros();
    dt = (time_now - time_prev) / 1000.0;
    time_prev = time_now;
    cnt = 0;
    Serial.println(dt);
  }
  */

  // データの送受信を行う(デフォルトは2ms周期)
  Cubic::update();
}
