#include <Servo.h>

//Servos
Servo panServo;
Servo tiltServo;
const int tiltPin = 13;
const int panPin = 12;
String x_str = "";
String y_str = "";
String r_str = "";
int panAngle = 1725;
int tiltAngle = 1800;
int servoEnd = 0;
int endx = 0;
int endy = 0;
int endr = 0;

//Linear Actuator 
const int feedback = A2; //potentiometer from actuator
const int cRaw = A5; //current reading for actuator
const int enable = 8;
const int PWMA = 11;
const int PWMB = 3;
int dest = 420;

bool r2fall = false;

//Linear Actuator goes from 420 - 1020
//tilt servo goes from 2000 (low) - 1700 (high) --after initial process starting from arm rest
//pan servo goes from 1950 (left) - 1500 (right)

void setup() {
  panServo.attach(10);
  tiltServo.attach(9);
  pinMode(tiltPin, OUTPUT);
  pinMode(panPin, OUTPUT);

  pinMode(feedback, INPUT);//feedback from actuator
  pinMode(cRaw, INPUT);//current from actuator
  pinMode(enable, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);//three pins for MegaMoto
  digitalWrite(enable,HIGH);

  Serial.begin(115200);
  while (!Serial) {
    delay(1000); // wait for serial port to connect
  } 

  digitalWrite(tiltPin, LOW); //turn off servo power
  digitalWrite(panPin, LOW);  
  panServo.write(panAngle);
  tiltServo.write(1600);

}


void loop() {

  int i, j, k;

  //Retract linear actuator
  actuatorMove(dest); 

  //Wait until receive connection from RasPi
  int in_char = Serial.read();
  while (in_char != 'b'){
    in_char = Serial.read();
  }

  //give servos power
  digitalWrite(tiltPin, HIGH);
  digitalWrite(panPin, HIGH);
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);

  delay(4000);
  
  panSearch(); //sweep search for color circle in frame of view
  serialRead(); //go through procedure of moving arm to v-envelope
  
  digitalWrite(panPin, LOW);
  delay(5000);

  //the following loop is the procedure once the arm is directly above the v envelope slot
  for( i=0; i<5; i++){
     tiltAngle+=15;
     tiltServo.write(tiltAngle);
     delay(200);
   }

   for ( i=0; i<4; i++){
     dest -= 20;
     if (dest>1020){
       dest = 1000;
     }
     actuatorMove(dest);
     delay(200);

     digitalWrite(panPin, HIGH); //turn on pan servo power
     for( j=0; j<6; j++){
       panAngle+=10;
       panServo.write(panAngle);
       delay (200);
     }
     for( k=0; k<6; k++){
       panAngle-=10;
       panServo.write(panAngle);
       delay (200);
     }
     digitalWrite(panPin, LOW);
     delay(200);
     
     for( j=0; j<3; j++){
       tiltAngle+=10;
       tiltServo.write(tiltAngle);
       delay(200);
     }

     dest += 20;
     if (dest>1020){
       dest = 1000;
     }
     actuatorMove(dest);
     delay(200);
     
   }
   
  digitalWrite(tiltPin, LOW); //turn off tilt servo power so it falls in v-envelope

  delay (30000000); 
}

void panSearch(){ //pan to search for ball

  int leftCheck = 0;
  int rightCheck = 0;
  int in_char = 'b';
  int i = 0;

  while (i<1000){
    delay(10);
    i++;
    in_char = Serial.read();
  } 
  
  while ((in_char != 'x') && (tiltAngle < 2100)){
    if (rightCheck == 0){
      if (panAngle < 1950) {
        panAngle += 1;
        panServo.write(panAngle);
        delay(10);
      }
      else {
        rightCheck = 1;
        tiltAngle+=15;
        tiltServo.write(tiltAngle);
      }
    }
    else if (leftCheck == 0){
      if (panAngle > 1500) {
        panAngle -= 1;
        panServo.write(panAngle);
        delay(10);
      }
      else {
        leftCheck = 1;
        tiltAngle+=15;
        tiltServo.write(tiltAngle);
      }
    }
    else {
      rightCheck = 0;
      leftCheck = 0;
    }
    
    in_char = Serial.read();
  }

  leftCheck = 0;
  rightCheck = 0;

}

void actuatorMove(int destination){
  int curr = 0; //current monitoring
  int difference = 0;//values for knowing location
  int precision = 1;//how close to final value to get
  int currentPosition = 0;
  int breakFlag = 0;
  int currCnt = 0;
  r2fall = false;

  if (destination > 1020){
    destination = 1020;
  }

  if (destination < 420){
    destination = 420;
  }
  
  while(breakFlag == 0){

    currentPosition = analogRead(feedback);//check where you are
    difference = destination - currentPosition;//find out how far you are from the destination

    if ((difference > precision) || (difference < -precision)){
      if (currentPosition > destination) {
        analogWrite(PWMB,0);
        analogWrite(PWMA,175);
      }
      else if (currentPosition < destination) {
        analogWrite(PWMB,175);
        analogWrite(PWMA,0);
      }
    }
    else {
      analogWrite(PWMA,0);
      analogWrite(PWMB,0);
      breakFlag = 1;
      r2fall = true;
    }
    delay(50);

  }
  
}

int rad2dest (int radius) { //converts the radius of the circle from openCV to a pwm command for the linear actuator
  float inches = ((-.583)*float(radius))+29.16;
  float destFloat = ((inches/(12.0))*600)+520;

  if (destFloat < 420){
    destFloat = 420;
  }
  
  dest = int(destFloat);
  
  return(dest);
}

void panControl(int x) {  
   if (x < 278){
      panAngle -= 1;
      if (panAngle < 1500){
        panAngle = 1500;
      }
   }
   else if (x > 282){
      panAngle += 1;
      if (panAngle > 1950){
        panAngle = 1950;
      }
   }
   else endx = 1;

   panServo.write(panAngle);
   return;
}

void tiltControl(int y) {   
   if (y < 413){
      tiltAngle -= 1;
      if (tiltAngle < 1700){
        tiltAngle = 1700;
      }
   }
   else if (y > 417){
      tiltAngle += 1;
      if (tiltAngle > 2000){
        tiltAngle = 2000;
      }
   }
   else endy = 1;

   tiltServo.write(tiltAngle);
   return;
}

void serialRead() {

  int in_char = Serial.read();

  while (servoEnd == 0)  {
    in_char = Serial.read();

    if (in_char == 'x'){
      while(in_char != '\n'){
        in_char = Serial.read();
        if (isDigit(in_char)) {
          x_str += (char)in_char;
        }
      }
      panControl(x_str.toInt());
      x_str = ""; // clear string for new input:     
    }

    if (in_char == 'y'){
      while(in_char != '\n'){
        in_char = Serial.read();
        if (isDigit(in_char)) {
          y_str += (char)in_char;
        }
      }
      tiltControl(y_str.toInt());
      y_str = ""; // clear string for new input:     
    }

    if ((endx == 1) && (endy == 1)) {
      servoEnd = 1;
      
    }
  }
  
  while(endr == 0) {
    in_char = Serial.read();
  
    if (in_char == 'r'){
      while(in_char != '\n'){
        in_char = Serial.read();
        if (isDigit(in_char)) {
          r_str += (char)in_char;
        }
      }
      actuatorMove(rad2dest(r_str.toInt()));
      r_str = ""; // clear string for new input:     
      endr = 1;
    }
  }

  servoEnd = 0;
  endx = 0;
  endy = 0;
  endr = 0;

  return;
  
}
