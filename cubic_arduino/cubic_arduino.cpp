#include "cubic_arduino.h"

SPISettings Cubic_SPISettings = SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0);
SPISettings ADC_SPISettings = SPISettings(ADC_SPI_FREQ, MSBFIRST, SPI_MODE0);

const uint8_t Adc::ch[DC_MOTOR_NUM] = {7, 5, 6, 4, 3, 2, 0, 1};

int16_t DC_motor::buf[DC_MOTOR_NUM+SOL_SUB_NUM];
int16_t DC_motor::buf_B[DC_MOTOR_NUM+SOL_SUB_NUM];
uint8_t Inc_enc::buf[INC_ENC_NUM*INC_ENC_BYTES];
uint8_t Abs_enc::buf[ABS_ENC_NUM*ABS_ENC_BYTES];
float Adc::buf[DC_MOTOR_NUM];

bool DC_motor::_use_B = false;
bool Solenoid::_use_B = false;
unsigned long Solenoid::time_prev[SOL_SUB_NUM*2];
int32_t Inc_enc::val_prev[INC_ENC_NUM];
unsigned long Cubic::time_prev;
float Adc::bias[DC_MOTOR_NUM];
float Adc::buf_prev[DC_MOTOR_NUM];
float Cubic::_current_limit;


void DC_motor::begin(bool use_B){
	_use_B = use_B;
    pinMode(SS_MD_A,OUTPUT);
    digitalWriteFast(Pin(SS_MD_A),HIGH);
	if(_use_B){
		pinMode(SS_MD_B,OUTPUT);
		digitalWriteFast(Pin(SS_MD_B),HIGH);
	}
    pinMode(SS_MD_MISO,OUTPUT);
    digitalWriteFast(Pin(SS_MD_MISO),HIGH);

    //マザーボード上のRP2040とモータドライバの各マイコン間でのSPI通信も可能
    pinMode(SS_MD_SS_A0,OUTPUT);
    pinMode(SS_MD_SS_A1,OUTPUT);
    pinMode(SS_MD_SS_A2,OUTPUT);
    pinMode(SS_MD_SS_A3,OUTPUT);
    digitalWriteFast(Pin(SS_MD_SS_A0),HIGH);
    digitalWriteFast(Pin(SS_MD_SS_A1),HIGH);
    digitalWriteFast(Pin(SS_MD_SS_A2),HIGH);
    digitalWriteFast(Pin(SS_MD_SS_A3),HIGH);
	if(_use_B){
		pinMode(SS_MD_SS_B0,OUTPUT);
		pinMode(SS_MD_SS_B1,OUTPUT);
		pinMode(SS_MD_SS_B2,OUTPUT);
		pinMode(SS_MD_SS_B3,OUTPUT);
		digitalWriteFast(Pin(SS_MD_SS_B0),HIGH);
		digitalWriteFast(Pin(SS_MD_SS_B1),HIGH);
		digitalWriteFast(Pin(SS_MD_SS_B2),HIGH);
		digitalWriteFast(Pin(SS_MD_SS_B3),HIGH);
	}
}

void DC_motor::put(const uint8_t num, const int16_t duty, const uint16_t duty_max){
    // 想定外の入力が来たら何もしない
    if(duty_max > DUTY_SPI_MAX) return;
    if(abs(duty) > duty_max) return;
	if(num >= (DC_MOTOR_NUM + SOL_SUB_NUM) * (_use_B ? 2:1)) return;

    // duty値を代入
	if(num < DC_MOTOR_NUM + SOL_SUB_NUM) buf[num] = (int16_t)((float)duty/(float)duty_max * (float)DUTY_SPI_MAX);
	else buf_B[num - DC_MOTOR_NUM - SOL_SUB_NUM] = (int16_t)((float)duty/(float)duty_max * (float)DUTY_SPI_MAX);
}

int16_t DC_motor::get(uint8_t num) {
	if(num >= (DC_MOTOR_NUM + SOL_SUB_NUM) * (_use_B ? 2:1)) return -1;

    if(num < DC_MOTOR_NUM + SOL_SUB_NUM) return buf[num];
	else return buf_B[num - DC_MOTOR_NUM - SOL_SUB_NUM];
}

void DC_motor::send(void){
    uint8_t *l_buf = (uint8_t*)buf;
    uint8_t sign_buf = 0;
    SPI.beginTransaction(Cubic_SPISettings);

    // A面の送信要求を受け取る
    digitalWriteFast(Pin(SS_MD_MISO),LOW);
    digitalWriteFast(Pin(SS_MD_A),LOW);
    sign_buf = SPI.transfer(0x00);
    digitalWriteFast(Pin(SS_MD_MISO),HIGH);
    digitalWriteFast(Pin(SS_MD_A),HIGH);
    delayMicroseconds(1);

    // 送信要求データ（2進数で"11111111"）だったならデータを送信***スレーブからマスターへのデータ送信はデータが破損（？）するのでそれに対する応急処置。要修正***
    if(sign_buf == 0xFF){
        for (int i = 0; i < (DC_MOTOR_NUM+SOL_SUB_NUM)*DC_MOTOR_BYTES; i++) {
            //digitalWriteFast(Pin(SS_MD_MISO),LOW);
            digitalWriteFast(Pin(SS_MD_A),LOW);
            SPI.transfer(l_buf[i]);
            digitalWriteFast(Pin(SS_MD_A),HIGH);
        }
    }
    SPI.endTransaction();

	// B面を使わない場合はここで終了
	if(!_use_B) return;
	l_buf = (uint8_t*)buf_B;
  sign_buf = 0;
    SPI.beginTransaction(Cubic_SPISettings);
    digitalWriteFast(Pin(ENABLE_MD_B),LOW);
    digitalWriteFast(Pin(SS_MD_B),LOW);
    sign_buf = SPI.transfer(0x00);
    digitalWriteFast(Pin(ENABLE_MD_B),HIGH);
    digitalWriteFast(Pin(SS_MD_B),HIGH);
    delayMicroseconds(1);
	if(sign_buf == 0xFF){
        for (int i = 0; i < (DC_MOTOR_NUM+SOL_SUB_NUM)*DC_MOTOR_BYTES; i++) {
            digitalWriteFast(Pin(SS_MD_B),LOW);
            SPI.transfer(l_buf[i]);
            digitalWriteFast(Pin(SS_MD_B),HIGH);
        }
    }
    SPI.endTransaction();
}

void DC_motor::print(const bool new_line){
    for (int i = 0; i < DC_MOTOR_NUM+SOL_SUB_NUM; i++) {
        if (abs(buf[i]) == DUTY_SPI_MAX + 1 && i >= DC_MOTOR_NUM) {
            Serial.print("SOL");
        }
        else {
            Serial.print(buf[i]);
        }
        Serial.print(" ");
    }
	// B面を使わない場合はB面のデータを出力しない
	if(_use_B){
    Serial.print("B-side: ");
		for (int i = 0; i < DC_MOTOR_NUM+SOL_SUB_NUM; i++) {
			if (abs(buf_B[i]) == DUTY_SPI_MAX + 1 && i >= DC_MOTOR_NUM) {
				Serial.print("SOL");
			}
			else {
				Serial.print(buf_B[i]);
			}
			Serial.print(" ");
		}
	}
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}


void Solenoid::begin(bool use_B) {
	_use_B = use_B;
    for (int i = 0; i < SOL_SUB_NUM*2; i++) {
        time_prev[i] = millis();
    }
}

void Solenoid::put(const uint8_t num, const bool state) {
    if (num >= SOL_SUB_NUM * (_use_B ? 2:1)) return;
	if (num < SOL_SUB_NUM) {
		if (DC_motor::buf[DC_MOTOR_NUM + num] == (state ? DUTY_SPI_MAX + 1 : -(DUTY_SPI_MAX + 1))) return;
    
		unsigned long time_now = millis();
		if (time_now - time_prev[num] < SOL_TIME_MIN) return;

		DC_motor::buf[DC_MOTOR_NUM+num] = (state ? DUTY_SPI_MAX + 1 : -(DUTY_SPI_MAX + 1));
	  	time_prev[num] = time_now;
	} else {
		if (DC_motor::buf_B[DC_MOTOR_NUM + num - SOL_SUB_NUM] == (state ? DUTY_SPI_MAX + 1 : -(DUTY_SPI_MAX + 1))) return;
		
		unsigned long time_now = millis();
		if (time_now - time_prev[num] < SOL_TIME_MIN) return;

		DC_motor::buf_B[DC_MOTOR_NUM + num - SOL_SUB_NUM] = (state ? DUTY_SPI_MAX + 1 : -(DUTY_SPI_MAX + 1));
	  	time_prev[num] = time_now;
	}
}

int8_t Solenoid::get(const uint8_t num) {
    if (num >= SOL_SUB_NUM * (_use_B ? 2:1)) return -1;
	int16_t raw_val;
	if (_use_B) {
		raw_val = DC_motor::buf[DC_MOTOR_NUM+num];
	}else{

		raw_val = DC_motor::buf_B[DC_MOTOR_NUM+num-SOL_SUB_NUM];
	}
    return (abs(raw_val) == DUTY_SPI_MAX + 1 ? (raw_val < 0 ? 0 : 1) : -1);
}

void Solenoid::print(const bool new_line) {
    for (int i = 0; i < SOL_SUB_NUM*(_use_B ? 2:1); i++) {
        int8_t val = get(i);
        if (val == -1) Serial.print("MOT");
        else           Serial.print(val);
        Serial.print(" ");
    }
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}


void Inc_enc::begin(void){
    pinMode(SS_INC_ENC, OUTPUT);
    digitalWriteFast(Pin(SS_INC_ENC), HIGH);

    pinMode(INC_ENC_RESET, OUTPUT);
    digitalWriteFast(Pin(INC_ENC_RESET), HIGH);
}

int32_t Inc_enc::get(const uint8_t num){
    if(num >= INC_ENC_NUM) return 1;

    int32_t ret = 0;
    ret |= buf[num*INC_ENC_BYTES];
    ret |= buf[num*INC_ENC_BYTES+1] << 8;
    ret |= buf[num*INC_ENC_BYTES+2] << 16;
    ret |= buf[num*INC_ENC_BYTES+3] << 24;
    return ret;
}

int16_t Inc_enc::get_diff(const uint8_t num){
    if(num >= INC_ENC_NUM) return 1;

    return get(num) - val_prev[num];
}

void Inc_enc::save_val(void){
    for (int i = 0; i < INC_ENC_NUM; i++) {
        val_prev[i] = get(i);
    }
}

void Inc_enc::receive(void){
    save_val();

    SPI.beginTransaction(Cubic_SPISettings);
    // データを受信
    for (int i = 0; i < INC_ENC_NUM*INC_ENC_BYTES; i++) {
        digitalWriteFast(Pin(SS_INC_ENC),LOW);
        buf[i] = SPI.transfer(0x88);
        digitalWriteFast(Pin(SS_INC_ENC),HIGH);
    }
    SPI.endTransaction();
}

void Inc_enc::reset(void){
    digitalWriteFast(Pin(INC_ENC_RESET), LOW);
    delayMicroseconds(1);
    digitalWriteFast(Pin(INC_ENC_RESET), HIGH);
}

void Inc_enc::print(const bool new_line){
    for (int i = 0; i < INC_ENC_NUM; i++) {
        Serial.print(get(i));
        Serial.print(" ");
    }
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}

void Inc_enc::print_diff(const bool new_line){
    for (int i = 0; i < INC_ENC_NUM; i++) {
        Serial.print(get_diff(i));
        Serial.print(" ");
    }
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}


void Abs_enc::begin(void){
    pinMode(SS_ABS_ENC, OUTPUT);
    digitalWriteFast(Pin(SS_ABS_ENC), HIGH);
}

uint16_t Abs_enc::get(const uint8_t num){
    if(num >= ABS_ENC_NUM) return ABS_ENC_ERR;

    uint16_t ret = 0;
    ret |= buf[num*ABS_ENC_BYTES];
    ret |= buf[num*ABS_ENC_BYTES+1] << 8;

    // RP2040で正しく読めてない場合
    if(ret == ABS_ENC_ERR_RP2040) return ret;

    if(!(parity_check(ret))) {
        // Arduinoで正しく読めてない場合
        return ABS_ENC_ERR;
    }
    else {
        // 正しく読めた場合
        return remove_parity_bit(ret);
    }
}

bool Abs_enc::parity_check(const uint16_t enc_val) {
    bool bit[16];
    for (int i = 0; i < 16; i++) {
        bit[i] = (enc_val >> i) & 1;
    }

    if (bit[15] == !(bit[13] ^ bit[11] ^ bit[9] ^ bit[7] ^ bit[5] ^ bit[3] ^ bit[1]) &&
        bit[14] == !(bit[12] ^ bit[10] ^ bit[8] ^ bit[6] ^ bit[4] ^ bit[2] ^ bit[0])) {
        return true;
    }
    return false;
}

uint16_t Abs_enc::remove_parity_bit(const uint16_t enc_val) {
    return enc_val & 0x3fff;
}

void Abs_enc::receive(void){
    SPI.beginTransaction(Cubic_SPISettings);

    // データを受信
    for (int i = 0; i < ABS_ENC_NUM*ABS_ENC_BYTES; i++) {
        digitalWriteFast(Pin(SS_ABS_ENC),LOW);
        buf[i] = SPI.transfer(0x88);
        digitalWriteFast(Pin(SS_ABS_ENC),HIGH);
    }
    SPI.endTransaction();
}

void Abs_enc::print(const bool new_line) {
    for (int i = 0; i < ABS_ENC_NUM; i++) {
        uint16_t val = get(i);
        if      (val == ABS_ENC_ERR)        Serial.print("ARD_ERR");
        else if (val == ABS_ENC_ERR_RP2040) Serial.print("RP2040_ERR");
        else                                Serial.print(val);
        Serial.print(" ");
    }
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}


void Adc::begin(void) {
    // ADCのSSの初期化
    pinMode(SS_ADC_A,OUTPUT);
    pinMode(SS_ADC_B,OUTPUT);
    digitalWriteFast(Pin(SS_ADC_A),HIGH);
    digitalWriteFast(Pin(SS_ADC_B),HIGH);
    
    // バイアス項を求める
    const int vrfy_num = 10;
    float temp[DC_MOTOR_NUM];
    for(int i = 0; i < DC_MOTOR_NUM; i++) bias[i] = temp[i] = 0.0;
    for(int i = 0; i < vrfy_num; i++) {
        receive();
        for(int j = 0; j < DC_MOTOR_NUM; j++) {
            temp[j] += buf[j];
        }
    }
    for(int i = 0; i < DC_MOTOR_NUM; i++) bias[i] = -temp[i] / vrfy_num;
}

float Adc::get(uint8_t num) {
    if (num >= DC_MOTOR_NUM) return 0;
    return buf[num];
}

void Adc::receive(void) {
    SPI.beginTransaction(ADC_SPISettings);
    for(int i = 0; i < DC_MOTOR_NUM; i++) {
        byte channelDataH2 = (ch[i] >> 2) | 0x06;
        byte channelDataL2 = ch[i] << 6;

        digitalWriteFast(Pin(SS_ADC_A), LOW);
        digitalWriteFast(Pin(SS_ADC_B), LOW);
        SPI.transfer(channelDataH2);                  // Start bit 1 + D2bit
        byte highByte = SPI.transfer(channelDataL2);  // singleEnd D1,D0 bit
        byte lowByte = SPI.transfer(0x00);            // dummy
        digitalWriteFast(Pin(SS_ADC_A), HIGH);
        digitalWriteFast(Pin(SS_ADC_B), HIGH);

        int data = ((highByte & 0x0f) << 8) | lowByte;
        float raw_val = (float)(data - CURRENT_RES)/CURRENT_RES * CURRENT_MAX + bias[i];
        buf[i] = 0.1*raw_val + 0.9*buf_prev[i]; // ローパスフィルタ
        buf_prev[i] = buf[i];
    }
    SPI.endTransaction();
}

void Adc::print(const bool new_line){
    for (int i = 0; i < DC_MOTOR_NUM; i++) {
        Serial.print(get(i));
        Serial.print(" ");
    }
    if (new_line == true)
        Serial.println();
    else
        Serial.print(" ");
}


void Cubic::begin(const bool use_B, const float current_limit){
    // Cubicの動作開始
    pinMode(ENABLE,OUTPUT);
    digitalWriteFast(Pin(ENABLE),HIGH);
    pinMode(ENABLE_MD_A, OUTPUT);
    digitalWriteFast(Pin(ENABLE_MD_A), HIGH);
    pinMode(ENABLE_MD_B, OUTPUT);
    digitalWriteFast(Pin(ENABLE_MD_B), HIGH);

    // SPI通信セットアップ
    SPI.begin();
    pinMode(CubicPin::MISO, INPUT_PULLUP);

    // DCモータの初期化
    DC_motor::begin(use_B);

    // ソレノイドの初期化
    Solenoid::begin(use_B);

    // インクリメントエンコーダの初期化
    Inc_enc::begin();

    // アブソリュートエンコーダの初期化
    Abs_enc::begin();

    // ADCの初期化
    Adc::begin();
    // 電流の許容値を設定
    _current_limit = abs(current_limit);

    // ループ前の時刻を記録
    time_prev = micros();

    Cubic::update();
    Cubic::update();
    Cubic::update();
}

void Cubic::update(const unsigned int us) {
    // ここに電流値によるチェックを入れる
    // for(int i = 0; i < DC_MOTOR_NUM; i++) {
    //     if(abs(Adc::get(i)) > _current_limit) {
    //         DC_motor::put(i, 0);
    //     }
    // }
    DC_motor::send();

    unsigned long time_now = micros();
    unsigned int dt;
    if(time_now < time_prev) dt = time_now + MICROS_MAX - time_prev;
    else                     dt = time_now - time_prev;
    time_prev = time_now;

    if(us > dt) delayMicroseconds((us - dt)*2); // なぜか2倍すると正しい周期になる

    Abs_enc::receive();
    Inc_enc::receive();
    Adc::receive();
}
