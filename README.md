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

SS ... Slave Select  
MISO ... Master In Slave Out  
MOSI ... Master Out Slave In  
SCK ... Serial Clock

## SPI通信について
### マザーボードRP2040との通信
マザーボードのRP2040が行っているのはモータとソレノイドの制御です。Cubicでは各チャンネルに指定したduty比を、Arduino33BLEからRP2040にSPI通信で渡しています。

まず33BLEの方でDC_motor::put()を用いてバッファに指定するduty比を保存します。  
バッファは24個要素を持った配列で、バッファの前半12個はA面のduty比、後半12個はB面のduty比を格納しています。  
各面の12個の要素は、最初の8個がメインチャンネルのduty比、後ろの4個がサブチャンネル(サブモータorソレノイド)のduty比を格納しています。

このバッファはB面を使用するか否かに拘わらず必ず要素24個分のメモリを確保します。  
なのでA面しか使わない状況でどうしてもメモリが足りなくなったらライブラリをいじってこのバッファの要素数を12個に減らしてください。(動的にメモリを確保してもいいですが、コンパイル時ではなく実行時にメモリが確保されることになり予測不可能性が生じると考えたためやっていません。いい方法があれば好きに変えてください)
