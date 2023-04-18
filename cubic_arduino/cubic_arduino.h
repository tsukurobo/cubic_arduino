#ifndef Cubic_h
#define Cubic_h
#include "Arduino.h"
#include <SPI.h>
#include "33BLE_digitalWriteFast.hpp"
using namespace nano33BLE_digitalWriteFast;

// 各種ENABLEをHIGHにすることによって動作開始
#define ENABLE 7
#define ENABLE_EX0 2
#define ENABLE_EX1 3

// SPI通信に用いるピン
#define MISO 12
#define MOSI 11
#define SCK 13
#define SS_DC_MOTOR 15  // DCモータ(ソレノイド)のSS
#define SS_DC_MOTOR_MISO 2 //DCモーターで、スレーブからマスターへデータを送る時LOWにしないといけないピン
#define SS_INC_ENC 20  // インクリメントエンコーダのSS
#define SS_ABS_ENC 1    // アブソリュートエンコーダのSS
#define SS_ADC 21       // ADCのSS
#define SS_DC_MOTOR_SS_1 16//マザーボードのRP2040をマスターとして各モータードライバ基盤に配置できるスレーブのSS
#define SS_DC_MOTOR_SS_2 17
#define SS_DC_MOTOR_SS_3 18
#define SS_DC_MOTOR_SS_4 19

#define SPI_FREQ 4000000
#define ADC_SPI_FREQ 1000000

// モータ，エンコーダの数
#define DC_MOTOR_NUM 8
#define INC_ENC_NUM 8
#define ABS_ENC_NUM 8
#define SOL_SUB_NUM 4 // ソレノイドとサブチャンネルDCモータの数

// 送受信するデータ1つ辺りのバイト数
#define DC_MOTOR_BYTES 2
#define INC_ENC_BYTES 4
#define ABS_ENC_BYTES 2

// SPI通信におけるDCモータのDutyの最大値
#define DUTY_SPI_MAX 32766

// 電流センサの取り得る最大電流値
#define CURRENT_MAX 30.0
// 電流センサの分解能(-2048 ~ 2048)
#define CURRENT_RES 2048

// アブソリュートエンコーダの取り得る最大値
#define ABS_ENC_MAX 16383
// RP2040でアブソリュートエンコーダが正しく読めなかったときに返す値
#define ABS_ENC_ERR_RP2040 0x7fff
// Arduinoでアブソリュートエンコーダが正しく読めなかったときに返す値
#define ABS_ENC_ERR 0xffff

// インクリメントエンコーダのRP2040のリセットピン(LOW:reset HIGH:start)
#define INC_ENC_RESET 0

// ソレノイドの出力を切り替える最小時間(ms)
#define SOL_TIME_MIN 10
// micros()で測れる最大時間
#define MICROS_MAX 0xffffffff

class DC_motor {
    public:
        // 初期化する関数
        static void begin(void);

        // 指定したモータのDutyを格納する関数
        // 第1引数：モータ番号0~11，第2引数：duty，第3引数：最大duty
        static void put(uint8_t num, int16_t duty, uint16_t duty_max = 1000);

        // 指定したモータのDutyを取得する関数
        // 第1引数：モータ番号0~11
        static int16_t get(uint8_t num);

        // すべてのモータのDutyをSPI通信で送信する関数
        static void send(void);

        // すべてのモータのDutyの値をSerial.print()で表示する関数
        // ソレノイドの状態を出力している場合はSOLと表示される
        static void print(bool new_line = false);

        // RP2040への送信データを格納する配列
        // ソレノイドを使用する場合も後ろ4つの要素を使用する
        static int16_t buf[DC_MOTOR_NUM+SOL_SUB_NUM];
};

class Solenoid {
    public:
        // 初期化する関数
        static void begin(void);

        // 指定したソレノイドの状態を格納する関数
        // 第1引数：ソレノイド番号0~3，第2引数：状態
        static void put(uint8_t num, bool state);

        // 指定したソレノイドの状態を取得する関数
        static int8_t get(uint8_t num);

        // すべてのソレノイドの状態をSerial.print()で表示する関数
        static void print(bool new_line = false);

    private:
        // 状態を変更した時刻を保存する配列
        static unsigned long time_prev[SOL_SUB_NUM];
};

class Inc_enc{
    public:
        // 初期化する関数
        static void begin(void);

        // エンコーダの累積値を取得する関数
        // 第1引数：エンコーダ番号
        static int32_t get(uint8_t num);

        // エンコーダの差分値を取得する関数
        // 第1引数：エンコーダ番号
        static int16_t get_diff(uint8_t num);

        // すべてのエンコーダの累積値をSPI通信で受信する関数
        static void receive(void);

        // すべてのエンコーダの累積値を0にする関数
        static void reset(void);

        // すべてのエンコーダの累積値をSerial.print()で表示する関数
        static void print(bool new_line = false);

        // すべてのエンコーダの差分値をSerial.print()で表示する関数
        static void print_diff(bool new_line = false);

    private:
        // RP2040からの受信データを格納する配列
        static uint8_t buf[INC_ENC_NUM*INC_ENC_BYTES];

        // 1つ前の値を格納する配列
        static int32_t val_prev[INC_ENC_NUM];

        // 今の値をval_prevに保存する関数
        static void save_val(void);
};

class Abs_enc{
    public:
        // 初期化する関数
        static void begin(void);

        // エンコーダの値を取得する関数
        // 第1引数：エンコーダ番号
        static uint16_t get(uint8_t num);

        // すべてのエンコーダの値をSPI通信で受信する関数
        static void receive(void);

        // すべてのエンコーダの値をSerial.print()で表示する関数
        static void print(bool new_line = false);

    private:
        // RP2040からの受信データを格納する配列
        static uint8_t buf[ABS_ENC_NUM*ABS_ENC_BYTES];

        // RP2040からの受信データのパリティチェックをする関数
        static bool parity_check(uint16_t);

        // RP2040からの受信データのパリティビットを取り除く関数
        static uint16_t remove_parity_bit(uint16_t);
};

class Adc {
    public:
        // 初期化する関数
        static void begin(void);

        // 電流値を取得する関数
        static float get(uint8_t num);

        // 電流値を受信する関数
        static void receive(void);

        // 各メインモータに対応した電流値を表示する関数
        static void print(bool new_line = false);
    
    private:
        // 各メインモータに対応した電流値を格納する配列
        static float buf[DC_MOTOR_NUM];

        // 各メインモータに対応したチャンネル番号を格納する配列
        static const uint8_t ch[DC_MOTOR_NUM];

        // 各メインモータに対応した電流値のバイアス項
        static float bias[DC_MOTOR_NUM];

        // 各メインモータに対応した電流値の前の値
        static float buf_prev[DC_MOTOR_NUM];
};

class Cubic{
    public:
        // すべてのモータ，エンコーダの初期化をする関数
        static void begin(float current_limit = 2.0);

        // データの送受信をまとめて行う関数
        static void update(unsigned int us = 4000);
    
    private:
        // 1つ前のループの時刻
        static unsigned long time_prev;

        // モータを止める電流の閾値
        static float _current_limit;
};

#endif
