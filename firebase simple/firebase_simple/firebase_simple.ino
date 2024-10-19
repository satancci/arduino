#include <IOXhop_FirebaseESP32.h>
#include <WiFi.h>

#define link "link"
#define secret "secret"
#define nome "nome_wifi"
#define senha "senha_wifi"

String meu[2]={"fale, meu mano!", "talvez sim"};
float varios[2]={298.92743, 19.94552565};
bool hab;

void setup() {
  WiFi.begin(nome, senha);
  Firebase.begin(link, secret);
  Serial.begin(9600);
  do{
    delay(400);
    Serial.print(".");
  }while(WiFi.status()!=WL_CONNECTED);
  Serial.println(" Wifi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  for (int i=0; i < 45; i++){
    Firebase.setInt("/Okay/09223/tempo", i);
    Firebase.setBool("/Okay/09223/habilitado", hab^=true);
    Firebase.setFloat("/Okay/09223/temperatura", varios[i%2], 2);
    Firebase.setString("/Okay/09223/msg", meu[i%2]);
    delay(500);
  }
}