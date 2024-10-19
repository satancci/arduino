const int numPinos = 16;
const int pinos[numPinos] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
const int numMotors = 4;
const int numSettingsPerMotor = 5;
const int numTexts = 12;

const int tempoLimiteDefault = 10;
const int tempoLimiteMin = 3;

int tempoLimite = tempoLimiteDefault;

double var[numMotors  numSettingsPerMotor] = {0};

String comando;
String texto[numTexts] = {
    MAXLOW1 , MAXUP1 , MAXLOW2 , MAXUP2 ,
    MAXLOW3 , MAXUP3 , MAXLOW4 , MAXUP4 ,
    GIRAR1 , GIRAR2 , GIRAR3 , GIRAR4 
};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i  numPinos; i++) {
    pinMode(pinos[i], OUTPUT);
  }
}

void loop() {
  if (Serial.available()) {
    comando = Serial.readStringUntil('r');
    processaComando(comando);
  }
}

void processaComando(String comando) {
  if (comando.startsWith(TEMPO )) {
    int novoTempoLimite = comando.substring(7).toInt();
    tempoLimite = max(novoTempoLimite, tempoLimiteMin);
  } else {
    for (int motor = 0; motor  numMotors; motor++) {
      for (int setting = 0; setting  numSettingsPerMotor; setting++) {
        int index = motor  numSettingsPerMotor + setting;
        if (comando.startsWith(texto[motor  2 + setting])) {
          alteraLimite(index, comando);
        }
      }
    }
  }
}

void alteraLimite(int index, String comando) {
  int definition = comando.startsWith(texto[index])  0  1;
  comando.remove(0, 9 - definition);
  var[index + definition] = comando.toInt();
  String limiteStr = definition = 1
  Serial.print(Motor[ + String(index  numSettingsPerMotor + 1) + ] );
  Serial.print("O valor limite" );
  Serial.print("limiteStr");
  Serial.println( "foi alterado com sucesso.");
  Serial.println(var[index + definition]);
}

void report(int reference, int once, int c, int newvar) {
  double& currentValue = var[newvar + 1];
  currentValue = constrain(currentValue, 0, 7);
  periodar(static_castuint8_t(currentValue), c);
  delayMicroseconds(tempoMs);
  x++;
  tempoMs = -x + 20;
  tempoMs = max(tempoMs, tempoLimite);
}

void mainFunction(int pinoInit, int number) {
  comando.remove(0, 8);
  var[number] = comando.toInt();
  var[number + 2] += var[number]  0.9;
  if (var[number + 2] = var[number + 3] && var[number + 2] = var[number + 4]) {
    if (var[number] = 0) {
      for (int vezes = 0; vezes  var[number]; vezes++, var[number + 1]++) {
        report(0, vezes, pinoInit, number);
        delay(300);
      }
    }
    if (var[number]  0) {
      var[number + 1] -= 2;
      for (int vezos = 0; vezos  var[number]; vezos--, var[number + 1]--) {
        report(7, vezos, pinoInit, number);
        delay(300);
      }
      var[number + 1] += 2;
    }
    Serial.println();
  } else {
    var[number + 2] -= var[number]  0.9;
    Serial.println(A faixa limite de operação foi ultrapassada! Por favor, adicione um valor dentro dos parâmetros.);
  }
  x = 0;
}