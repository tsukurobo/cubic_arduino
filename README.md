# cubic_arduino
このリポジトリは、統合基盤CubicをArduinoで制御するためのライブラリとそのサンプルです。

## ピンアサイン
|ピン番号|対応する定数名|役割|
|:---:|:---:|:---:|
|D1|SS_ABS_ENC|アブソリュートエンコーダ用RP2040のSS|
|D2|ENABLE_MD_A|A面のENABLE|
|D3|ENABLE_MD_B|B面のENABLE|
|D4|SS_MD_SS_B0|B面のモータドライバのSS|
|D5|SS_MD_SS_B1|〃|
|D6|SS_MD_B|B面のマザーボードRP2040のSS|
|D7|ENABLE|CubicのENABLE|
|D8|SS_MD_SS_B2|B面のモータドライバのSS|
|D9|SS_MD_SS_B3|〃|
|D10|SS_ADC_B|B面の電流計測用ADコンバータのSS|
|D11|MOSI|SPIのMOSI|
|D12|MISO|SPIのMISO|
|D13|SCK|SPIのSCK|
|D14(A0)|No Connection|未使用|
|D15(A1)|SS_MD_A|A面のマザーボードRP2040のSS|
|D16(A2)|SS_MD_SS_A0|A面のモータドライバのSS|
|D17(A3)|SS_MD_SS_A1|〃|
|D18(A4)|SS_MD_SS_A2|〃|
|D19(A5)|SS_MD_SS_A3|〃|
|D20(A6)|SS_INC_ENC|インクリメンタルエンコーダ用RP2040のSS|
|D21(A7)|SS_ADC_A|A面の電流計測用ADコンバータのSS|

SS...Slave Select
MISO...Master In Slave Out
MOSI...Master Out Slave In
SCK...Serial Clock