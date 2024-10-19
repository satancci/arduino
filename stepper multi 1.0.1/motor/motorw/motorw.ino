const unsigned int pinos[16] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
int direct, passo = 1, x, tempoLimite = 10;
double myAngle, maxUp = 360, maxLow = -360, tempoMs = 20, direct2, passo2=1,
 myAngle2, maxLow2 = -360, maxUp2=360, direct3, passo3=1, myAngle3, maxLow3 = -360,
 maxUp3=360, direct4, passo4=1, myAngle4, maxLow4 = -360, maxUp4=360;
double var[20] = {direct, passo, myAngle, maxLow, maxUp,
 direct2, passo2, myAngle2, maxLow2, maxUp2,
 direct3, passo3, myAngle3, maxLow3, maxUp3,
 direct4, passo4, myAngle4, maxLow4, maxUp4};
String comando;
String texto[12] = {"MAXLOW1: ", "MAXUP1: ", "MAXLOW2: ", "MAXUP2: ",
 "MAXLOW3: ", "MAXUP3: ", "MAXLOW4: ", "MAXUP4: ", "GIRAR1: ", "GIRAR2: ", "GIRAR3: ", "GIRAR4: "};

//==================================================================================================
void setup() {
  Serial.begin(9600);
  for (int a=0; a<16; a++){pinMode(pinos[a], OUTPUT);}
}
void loop() {
  //Verifica se a serial recebeu algo
  if(Serial.available()){
    comando = Serial.readStringUntil('\r');    
    //Diferencia a finalidade do comando
    for(int repeat = 0; repeat < sizeof(var)/20; repeat++){
      if(comando.startsWith(texto[repeat+8])){
        mainFunction(repeat*4,repeat*5);
      }
      limites(repeat+1, 0);
      limites(repeat+1, 1);
    }
    if(comando.startsWith("TEMPO: ")){
      comando.remove(0,7);
      tempoLimite = constrain(comando.toInt(), 5, 25);
    }
  }
}
void limites (uint8_t item, uint8_t definition){
  if(comando.startsWith(texto[(item*2) -2 + definition])){
      comando.remove(0, 9-definition);
      (var[3+definition+(5*(item-1))]) = comando.toInt();
      Serial.print("Motor[" + String(item) + "]: o valor limite ");
      definition == 1 ? Serial.print("maximo"): Serial.print("minimo");
      Serial.println(" foi alterado com sucesso.");
      Serial.println((var[3+definition+(5*(item-1))]));
    }
}
void periodar (uint8_t caso, int b){
  switch (caso){
    case 0: passar(1,0,0,0,b);
    break;
    case 1: passar(1,1,0,0,b);
    break;
    case 2: passar(0,1,0,0,b);
    break;
    case 3: passar(0,1,1,0,b);
    break;
    case 4: passar(0,0,1,0,b);
    break;
    case 5: passar(0,0,1,1,b);
    break;
    case 6: passar(0,0,0,1,b);
    break;
    case 7: passar(1,0,0,1,b);
    break;
  }
}
void passar (bool stateOne, bool stateTwo, bool stateTh, bool stateFour, int a){
  digitalWrite(pinos[a], stateOne);
  digitalWrite(pinos[a+1], stateTwo);
  digitalWrite(pinos[a+2], stateTh);
  digitalWrite(pinos[a+3], stateFour);
}
void report(int reference, int once, int c, int newvar){
  (var[newvar+1]) > 7 || (var[newvar+1]) < 0 ? (var[newvar+1]) = reference: (var[newvar+1]) = (var[newvar+1]);
  periodar((var[newvar+1]), c);
  delayMicroseconds(tempoMs);
  x++;
  tempoMs = max(-x+ 25, tempoLimite);
}
void mainFunction(int pinoInit, int  number){
  comando.remove(0,8);
  (var[number]) = comando.toInt();
  (var[number+2]) +=  (var[number])*0.9;
  if((var[number+2]) >= (var[number+3]) && (var[number+2]) <= (var[number+4])){
    //Gira no Sentido Positivo (HORÁRIO)
    if((var[number])>=0){
      for(int vezes = 0; vezes < (var[number]); vezes++, (var[number+1])++){
        report(0, vezes, pinoInit, number);
        delay(300);
      }
    }
    //Gira no Sentido Negativo (ANTI-HORÁRIO)
    if((var[number]) < 0){
      //Correção da inversão de sentido de rotação do motor
      (var[number+1])-=2;
      for(int vezos=0; vezos > (var[number]); vezos--, (var[number+1])--){
        report(7, vezos, pinoInit, number);
        delay(300);
      }
    //Reajuste da inversão de sentido de rotação do motor
    (var[number+1])+=2;
    } 
    Serial.println("|");
  }else{
    (var[number+2]) -=  (var[number])*0.9;
    Serial.println("A faixa limite de operação foi ultrapassada! Por favor, adicione um valor dentro dos parâmetros.");
  }
  x = 0;
}