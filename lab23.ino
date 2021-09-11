
#include <WiFi.h>
#include <HTTPClient.h>
#include <ThingSpeak.h>

#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 4
#define ERRPIN 12
#define ERRPIN2 27
#define DHTTYPE DHT22 
DHT dht(DHTPIN,DHTTYPE);


#define LM35PIN 34
#define BUZPIN 5
bool buz_fl=true;


char* ssid = (char *)"xxxxx"; // wifi name
char* pass = (char *)"xxxxx"; // wifi password
char* server = (char *)"api.thingspeak.com";
String WriteKey="xxxxxxxxxxxxxxxx"; // write api key
String write_to_thinkspeak(float a,float b,float c,int d,float e,float f){
    String re="https://api.thingspeak.com/update?api_key="+WriteKey+"&field1="+String(a, 3)+"&field2="+String(b, 3)+"&field3="+String(c, 3)+"&field4="+String(d, 3)+"&field5="+String(e, 3)+"&field6="+String(f, 3);
    return re;
}
bool ideal_humidity(float a,float b){
  float c=5.018+0.32321*b+0.0081847*b*b+0.00031243*b*b*b;
  if(abs(a-c)<20.0)return true;
  return false;
}
WiFiClient  client;
HTTPClient http;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid,pass);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  dht.begin();
  pinMode(BUZPIN, OUTPUT);
  pinMode(ERRPIN, OUTPUT);
  pinMode(ERRPIN2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  digitalWrite(ERRPIN, LOW);
  digitalWrite(ERRPIN2, LOW);
  
  if(client.connect(server,80)){
    float h=dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if(isnan(h)||isnan(t)||isnan(f)){
      Serial.println("DHT error");
    }
    else{
      int adc=analogRead(LM35PIN);
      float mv=adc*(3300.0)/4096.0;
      float t2=mv/10;
      float f2=t2*9/5+32;
      bool safe=ideal_humidity(h,t);
      int cc=0;
      if(safe==false)cc=1;
      http.begin(write_to_thinkspeak(h,t,f,cc,t2,f2));
      int httpResponse = http.GET();
       if (httpResponse>0) {   
          if(safe==false){
            digitalWrite(ERRPIN2, HIGH);
            if(buz_fl==true){
              digitalWrite(BUZPIN, HIGH);
              delay(1000);
              digitalWrite(BUZPIN, LOW);
            }
          }
          Serial.println("HTTP Response code: ");
          Serial.print(httpResponse);
          String payload = http.getString();
          Serial.println(payload);
        }
        else {
          digitalWrite(ERRPIN, HIGH);
          Serial.print("Error code: ");
          Serial.println(httpResponse);
        }
        http.end();
    }
      delay(10000);
  }
  
  client.stop();
  Serial.println("Waiting...");
  delay(1000);
}
