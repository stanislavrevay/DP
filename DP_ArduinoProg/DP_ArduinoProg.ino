#define stepsPerRevolution 400
#define dirPin 11 //CW+
#define stepPin 10 //CLK+
#define sensor 8 //senzor polohy
#define prepinac 12 //Prepinac manualneho modu

//Premenne na ovladanie motora
char serialCode = '0'; //Byte!!!
int serialCodeInt = 0; //Integer
int motorSpeed = 1600;
int distance = 0;

//Premenne na ovladanie (meranie) akcelerometrov
int i = 0;
const int Z1_axis = A0;
const int Z2_axis = A1;
int z1_adc_value = 0;
int z2_adc_value = 0;
float z1_g_value = 0;
float z2_g_value = 0;
int dobaMerania = 400;
bool zmenaDobyMerania = false;

void setup() {
  
  pinMode(9,OUTPUT);    //Mam zapojeny aj tento PIN,
  digitalWrite(9,LOW);  //preto tam dam pre istotu 0V
  pinMode(sensor,INPUT_PULLUP); //INPUT_PULLUP kvoli ruseniam
  pinMode(prepinac,INPUT_PULLUP);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  digitalWrite(dirPin, HIGH); //urcenie smeru otacania

  Serial.begin(230400); //POZOR NA BaudRate
}

void loop() {

  //Nastavenie a spustanie merania seriovou komunikaciou
  if (Serial.available()) {
    serialCode = Serial.read();
    serialCodeInt = (int)(serialCode-'0');
    if(serialCodeInt == 1) {
      Serial.print(1);  //Odosielam cislo je pre potvrdenie prijatia prikazu
    }
    
    if(serialCodeInt == 2) {  //Kontinualne meranie bez prerusenia
      ContinualMeasure();
    }

    if(serialCodeInt == 3){   //Meranie s ustalenim sa hodnot pred padom kolesa
      dobaMerania = dobaMerania + 100;
      StepMeasure();
    }

    if(zmenaDobyMerania == true){
      dobaMerania = serialCodeInt*100;
      zmenaDobyMerania = false;
      Serial.print(serialCodeInt);  //Odosielam cislo je pre potvrdenie prijatia prikazu
    }

    if(serialCodeInt == 0){
      zmenaDobyMerania = true;
      Serial.print(0);  //Odosielam cislo je pre potvrdenie prijatia prikazu
    }
  }

  //Spustenie merania tlacidlom
  if(digitalRead(prepinac) == 0){
    delay(30);
    if(digitalRead(prepinac) == 0){
      dobaMerania = dobaMerania + 100;
      StepMeasure();
    }
  }
}

void ContinualMeasure() {
  preparePosition();  //Priprava polohy vacky 
  //Zaciatok meranie...namiesto delayMicroseconds(motorSpeed*0.4) davam meranie, ktore pri motorSpeed = 1600 trva rovnako dlho
  measure();
  delayMicroseconds(250);
  while (distance <= 100) { // nasledne sa otacam rychlo a umoznim kolesu volny pad
    digitalWrite(stepPin, HIGH);
    measure();
    digitalWrite(stepPin, LOW);
    measure();
    distance = distance + 1;
  }

  while ( i < dobaMerania) {
    measure();
    i++;
    delayMicroseconds(250);
  }
  i = 0;
}

void StepMeasure(){
  preparePosition();
  //Cakanie na ustalenej hodnote
  i = 0;
  while(i < 200){   //Doba trvania Serial.print je cca 750us + 250us = 1ms
    measure();
    delayMicroseconds(250);
    i++;
  }
  i = 0;

  while (distance <= 100) { // nasledne sa otacam rychlo a umoznim kolesu volny pad
    digitalWrite(stepPin, HIGH);
    measure();
    digitalWrite(stepPin, LOW);
    measure();
    distance = distance + 1;
  }

  while ( i < dobaMerania) {
    measure();
    i++;
    delayMicroseconds(250);
  }
  i = 0;
}

void preparePosition() {
  
  if(digitalRead(sensor) == 0){ //Odchod z Home pozicie
    while (digitalRead(sensor) == 0) { // otacam sa pomaly, kym neodidem zo snimaca
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
    } 
  }
  while (digitalRead(sensor) == 1) { // otacam sa pomaly, kym nenarazim na snimac
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
  }
  distance = 0;
}

void measure(){
  z1_adc_value = analogRead(Z1_axis);
  z2_adc_value = analogRead(Z2_axis);

  //Acceleration in G
  z1_g_value = -((float)z1_adc_value - 328)/67*9.81;                        //Meranim som zistil, ze acc 9.81 na osi Z dosiahnem pri hodnote 397 a acc 0 pri hodnote 330...397-330=67
  z2_g_value = -((float)z2_adc_value - 326)/67*9.81;
  Serial.println(String( String(z1_g_value) + " " + String(z2_g_value)));
}
