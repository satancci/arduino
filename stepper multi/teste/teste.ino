long x[]={0,1,2,3,4,5};
long y[]={0,1,2,3,4,5};
long z[]={0,1,2,10,6,0};
String comando;
bool finalizado;
int once = 1;
void setup() {
 Serial.begin(9600);
 delayMicroseconds(3);
}
void loop() {
  if(!finalizado){
    for(once; once < sizeof(x) / sizeof(x[0]); once++){
      function("GIRAR1: ", x[once] - x[once-1]);
      function("GIRAR2: ", y[once] - y[once-1]);
      function("GIRAR3: ", z[once] - z[once-1]);
      function("GIRAR4: ", z[once] - z[once-1]);
    } 
    finalizado = true;
  }
}
void function(String texto, int vetor){
  Serial.print(texto);
  Serial.println(vetor);
    do {
      if(Serial.available()){
        comando = Serial.readStringUntil('\r');  
      }
    }while(comando!="|");
    comando = "";
}