uint8_t pinos[4] = {2,3,4,5};
int direct, passo = 1, x;
double myAngle, maxUp = 360, maxLow, tempoMs = 20;

//=====================================================================
void setup() {
  Serial.begin(9600);
  for (int a=0; a<4; a++){pinMode(pinos[a], OUTPUT);}
}

//======================================================================
void loop() {
  //Verifica se a serial recebeu algo
  if(Serial.available()){
    String comando = Serial.readStringUntil('\r');    
    //Diferencia a finalidade do comando
    if(comando.startsWith("GIRAR: ")){
      comando.remove(0,7);
      direct = comando.toInt();
      myAngle +=  direct*0.9;
      if(myAngle > maxLow && myAngle < maxUp){
        //Gira no Sentido Positivo (HORÁRIO)
        if(direct >= 0){
          for(int vezes = 0; vezes < direct; vezes++, passo++){
            report(0, vezes);
            delay(300);
          }
        }
        //Gira no Sentido Negativo (ANTI-HORÁRIO)
        if(direct < 0){
          //Correção da inversão de sentido de rotação do motor
          passo=passo-2;
          for(int vezos=0; vezos > direct; vezos--, passo--){
            report(7, vezos);
            delay(300);
          }
        //Reajuste da inversão de sentido de rotação do motor
        passo=passo+2;
        } 
      }else{
        myAngle -=  direct*0.9;
        Serial.println(myAngle);
        Serial.println("A faixa limite de operação foi ultrapassada! Por favor, adicione um valor dentro dos parâmetros.");
      }
      x = 0;
    }
    //Diferencia a finalidade do comando
    /*if(comando.startsWith("TEMPO: ")){
      comando.remove(0,7);
      tempoMs = comando.toInt();
      tempoMs < 5? tempoMs = 5: tempoMs = tempoMs;
    }*/
    if(comando.startsWith("MAXUP: ")){
      comando.remove(0,7);
      maxUp = comando.toInt();
      Serial.println("O valor limite maximo foi alterado com sucesso.");
    }
    if(comando.startsWith("MAXLOW: ")){
      comando.remove(0,8);
      maxLow = comando.toInt();
      Serial.println("O valor limite minimo foi alterado com sucesso.");
    }
  }
  delay(1000);
}

void periodar (uint8_t caso){
  switch (caso){
    case 0: passar(1,0,0,0);
    break;
    case 1: passar(1,1,0,0);
    break;
    case 2: passar(0,1,0,0);
    break;
    case 3: passar(0,1,1,0);
    break;
    case 4: passar(0,0,1,0);
    break;
    case 5: passar(0,0,1,1);
    break;
    case 6: passar(0,0,0,1);
    break;
    case 7: passar(1,0,0,1);
    break;
  }
}

void passar (bool stateOne, bool stateTwo, bool stateTh, bool stateFour){
  digitalWrite(pinos[0], stateOne);
  digitalWrite(pinos[1], stateTwo);
  digitalWrite(pinos[2], stateTh);
  digitalWrite(pinos[3], stateFour);
}

void report(int reference, int once){
  passo > 7 || passo < 0 ? passo = reference: passo = passo;
  periodar(passo);
  Serial.print("Passo: ");
  Serial.println(passo);
  Serial.print("Vezes: ");
  Serial.println(abs(once)+1);
  Serial.print("Direction: ");
  Serial.println(direct);
  Serial.print("Angle: ");
  Serial.println(myAngle);
  Serial.println("=====================");
  delayMicroseconds(tempoMs);
  x++;
  tempoMs = -x + 20;
  tempoMs < 5? tempoMs = 5: tempoMs = tempoMs; 
}