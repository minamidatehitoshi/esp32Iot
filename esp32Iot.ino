// ESP32を使った簡単なIoT
// https://qiita.com/northVil/items/fbdb2c3a52ebc32eaf7f

#include <WiFi.h>
#include <WebServer.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#define LED_PIN 2 // BUILTIN_LED
#define BTN_PIN 14  // Input SW
//Adafruit_BME280 bme;

float temp;
float pressure;
float humid;

int sensorPin = A7;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
String StateMsg = "---";
String BGColor = "white";

int sensor_sw = 1;

//const char *ssid = "Buffalo-A-aaaa";        // WifiのSSID
const char *ssid = "aterm-e96fa7-a";        // WifiのSSID
//const char *pass = "abcdefghijklm";    //  Wifiのパスワード
const char *pass = "33d30d0e290a9";    //  Wifiのパスワード
WebServer Server(80);         //  ポート番号（HTTP）

const char* ntpServer = "pool.ntp.org"; //NTPサーバー
const long  gmtOffset_sec =  9 * 3600; //時差（秒）
const int   daylightOffset_sec = 0; //夏時間の修正時差（秒）
unsigned long period = 1 * 3600 * 1000; // 時刻校正の周期（ミリ秒）
unsigned long rst = 0;

// NTP時刻を取得
void setNTP(){
  Serial.println("Swt Up ntp time");
  //時刻を取得
  //Serial.println("Set time");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

//  クライアントにウェブページ（HTML）を返す関数
void SendMessage() {
  //  レスポンス文字列の生成（'\n' は改行; '\' は行継続）
  //Serial.println("SendMessage");
  String dataString;
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){ // 現在時刻
    char cTime[256] = {'\0'};
    strftime(cTime, sizeof(cTime), "%Y/%m/%d %H:%M:%S", &timeinfo);
    dataString += String(cTime);
  }else{
    Serial.println("Failed to obtain time");
  }
  
  float voltage = 3.3/4096 * (float)sensorValue;
  String message =   "<html lang=\"ja\">\n\
    <meta charset=\"utf-8\" http-equiv=\"Refresh\" content=\"1\">\n\
    <center>\
    <h2>\
      <font size=+6>利用状況[eight3F]</font>\
    </h2>\
    <h3>\
      <font size=+3>" + String(dataString) + "</font>\
    </h3>\
    <p>\
      <button type='button' onclick='location.href=\"/\"' \
        style='width:750px;height:140px;background-color: " + String(BGColor) + ";color:#F0F0F0' \
        >\
        <font size=+6>" + String(StateMsg) + " </font>\
      </button>\
    </p>\
    </center>";
  //  クライアントにレスポンスを返す
  Server.send(200, "text/html", message);
}

// <button type='button' onclick='location.href=\"/bme\"' \

/*
void BmeSendMessage() {
  Serial.println("BmeSendMessage");
//  temp = bme.readTemperature();
//  humid = bme.readHumidity();
//  pressure = bme.readPressure();
  temp = 0;
  humid = 0;
  pressure = 1000;

  Serial.println("気温　" + String(temp));
  Serial.println("湿度　" + String(humid));
  Serial.println("気圧　" + String(pressure / 100));

  String message =   "<html lang=\"ja\">\n\
    <meta charset=\"utf-8\">\n\
    http-equiv=\"Refresh\" content=\"60\">\n\
    <center>\
    <h2>BME280の値</h2>\
    <h3>温度：　" + String(temp) + "°C, 湿度：　" + String(humid) + "%, 気圧：　" + String(pressure / 100) + "hPa</h3>\
    <p><button type='button' onclick='location.href=\"/bme\"' \
      style='width:250px;height:40px;'>気温・湿度・気圧を知る</button></p>\
  </center>";
  //  クライアントにレスポンスを返す
  Server.send(200, "text/html", message);
}
*/

//  クライアントにエラーメッセージを返す関数
void SendNotFound() {
  Serial.println("SendNotFound");
  Server.send(404, "text/plain", "404 not found...");
}

//  メインプログラム
void setup() {
  pinMode(LED_PIN, OUTPUT); // GPIO2を出力設定に
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  //  シリアルモニタ（動作ログ）
  Serial.begin(115200);               //  ESP 標準の通信速度 115200
  delay(100);                         //  100ms ほど待ってからログ出力可
  Serial.println("\n*** Starting ***");
  //  無線 LAN に接続
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);            
  Serial.println("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Can't connect");
    }
  }
  Serial.println("Connected");
  setNTP(); // 現在時刻取得
  Serial.println(WiFi.localIP());     //  ESP 自身の IP アドレスをログ出力
  //  ウェブサーバの設定
  Server.on("/", SendMessage);         //  ルートアクセス時の応答関数を設定
  //Server.on("/bme", BmeSendMessage);
  Server.onNotFound(SendNotFound);  //  不正アクセス時の応答関数を設定
  Server.begin();                     //  ウェブサーバ開始
  //BME280
  bool status = true;
  //status = bme.begin(0x76);

  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  while (!status) {
    Serial.println("BME280 sensorが使えません");
    delay(1000);
  }
  digitalWrite(LED_PIN, HIGH);
  //sensorValue = analogRead(sensorPin);
  //Serial.printf("analog = %d\n",sensorValue);
}
void loop() {
  sensor_sw = digitalRead(BTN_PIN);
  sensorValue = analogRead(sensorPin);
  //Serial.printf("analog = %d\n",sensorValue);
  //if(sensorValue > 0){
  if(sensor_sw > 0){
    BGColor = "blue";
    StateMsg = "Vacant";
  }
  else{
    BGColor = "red";
    StateMsg = "Occupied";
  }
 
  //  クライアントからの要求を処理する
  Server.handleClient();
}
