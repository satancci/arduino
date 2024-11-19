// Conexão Wifi lib
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// Firebase lib
#include <IOXhop_FirebaseESP32.h>

// Configurações para conexão
#define nome_wifi "seu-nome-do-wifi"
#define senha_wifi "sua-senha-do-wifi"
#define link_banco "seu-link-do-firebase"
#define senha_banco "seu-secret-do-firebase"

// Variáveis
int readMq2, readMq5, readMq9;
int entradas[3][2] = { { 18, readMq2 }, { 21, readMq5 }, { 25, readMq9 } };
String palavras[3][2] = { { "MQ-2", "Inflamavel" }, { "MQ-5", "Combustivel" }, { "MQ-9", "Toxico" } };
String ip = "";

void setup() {
  delay(1000);
  Serial.begin(9600);
  ip = conectar_wifi();
  ip = conectar_banco(ip);
  for (int i = 0; i < sizeof(entradas) / sizeof(entradas[0]); i++) pinMode(entradas[i][0], INPUT);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    entradas[i][1] = digitalRead(entradas[i][0]);
    Serial.println("[" + palavras[i][0] + "]: " + entradas[i][1]);
    Firebase.setBool("Dispositivos/" + ip + "/" + palavras[i][1], entradas[i][1]);
  }
  delay(1000);
}

String conectar_wifi() {
  WiFi.begin(nome_wifi, senha_wifi);
  Serial.print("Iniciando tentativa de conexao a rede " + String(nome_wifi));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wifi!\n IP: " + WiFi.localIP().toString());
  return WiFi.localIP().toString();
}

String conectar_banco(String myip) {
  myip.replace(".", "-");
  Firebase.begin(link_banco, senha_banco);
  Firebase.setBool("/Dispositivos/" + myip + "/Ligado", true);
  if (Firebase.failed()) {
    Serial.print("Erro na conexao com o Firebase: ");
    Serial.println(Firebase.error());
    while (true) {
      Serial.println("Por favor, reinicie o dispositivo.");
      delay(5000);
    }
  }
  Serial.println("Conectado ao Firebase!");
  return myip;
}
