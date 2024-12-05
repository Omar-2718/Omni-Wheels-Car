#include <Arduino.h>
#include <AFMotor.h>

// 45 degrees in radians
#define P45 0.78539816339

// initialize DC motors
AF_DCMotor m1(1), m2(2), m3(3), m4(4);

long double _sin, _cos, _max, theta;

void setup() {
  Serial.begin(9600);
}

int m = 0;      // magnitude of motion
int a = 0;      // angle of motion
int x = 0;      // rotational control
bool rot = false;  // flag to check if rotation is required

//read data from bluetooth device
void readMsg() {
  String receivedData = Serial.readStringUntil(';');
  
  if (receivedData.startsWith("X") && receivedData.indexOf("Y") != -1) {
    x = abs(receivedData.substring(1, receivedData.indexOf(',')).toInt()); 
    Serial.println(x);
    
    if (x >= 55) {
      rot = true;
    } else if (x <= 45) {
      rot = true;
    } else {
      rot = false;
    }
  }

  if (!rot && receivedData.startsWith("R") && receivedData.indexOf("A") != -1) {
    int commaIndex = receivedData.indexOf('A');

    m = abs(receivedData.substring(1, receivedData.indexOf(',')).toInt());
    a = receivedData.substring(commaIndex + 1, receivedData.length()).toInt();

    //calculations for motor control
    calc();
  }
}

// function to calculate sine and cosine values based on angle and normalize motor speeds
void calc() {
  theta = a * PI / 180;       // angle to radians
  _sin = sin(theta + P45);  
  _cos = cos(theta + P45); 
  _max = max(abs(_sin), abs(_cos));
}

//track the time since last command
unsigned long int timePassed = 0;

void loop() {
  //stop all motors if no command received for 5 seconds
  //this is to insure that if bluetooth connection is lost the car stops
  if (millis() - timePassed >= 5000) {
    m1.run(RELEASE);
    m2.run(RELEASE);
    m3.run(RELEASE);
    m4.run(RELEASE);
    m = 0;
  }

  int pow = 253;             
  int power_1, power_2, power_3, power_4;

  //check incoming serial data and process it
  if (Serial.available() > 0) {
    timePassed = millis();
    readMsg();   
  }

  //if not in rotation mode calculate movemnt mode varibles
  if (!rot) {
    power_1 = pow * m / 100 * _sin / _max;
    power_2 = pow * m / 100 * _cos / _max;
    power_3 = pow * m / 100 * _cos / _max;
    power_4 = pow * m / 100 * _sin / _max;

    int veps = 10; //threshold for minimum speed (to prevent jitter)

    //motor 1
    if (power_1 >= 0) {
      m1.setSpeed(power_1);
      if (power_1 < veps) m1.setSpeed(0);
      m1.run(FORWARD);
    } else {
      m1.setSpeed(-power_1);
      if (power_1 > -veps) m1.setSpeed(0);
      m1.run(BACKWARD);
    }

    //motor 2
    if (power_2 >= 0) {
      m2.setSpeed(power_2);
      if (power_2 < veps) m2.setSpeed(0);
      m2.run(FORWARD);
    } else {
      m2.setSpeed(-power_2);
      if (power_2 > -veps) m2.setSpeed(0);
      m2.run(BACKWARD);
    }

    //motor 3
    if (power_3 >= 0) {
      m3.setSpeed(power_3);
      if (power_3 < veps) m3.setSpeed(0);
      m3.run(FORWARD);
    } else {
      m3.setSpeed(-power_3);
      if (power_3 > -veps) m3.setSpeed(0);
      m3.run(BACKWARD);
    }

    //motor 4
    if (power_4 >= 0) {
      m4.setSpeed(power_4);
      if (power_4 < veps) m4.setSpeed(0);
      m4.run(FORWARD);
    } else {
      m4.setSpeed(-power_4);
      if (power_4 > -veps) m4.setSpeed(0);
      m4.run(BACKWARD);
    }
  } else {
    if (x <= 47) { //counter-clockwise
      int speed = 254 * (47 - x) / 47;
      m1.setSpeed(speed);
      m2.setSpeed(speed);
      m3.setSpeed(speed);
      m4.setSpeed(speed);
      m1.run(BACKWARD);
      m3.run(BACKWARD);
      m2.run(FORWARD);
      m4.run(FORWARD);
    } else if (x >= 57) { //clockwise
      int speed = 254 * (x - 57) / 43;
      m1.setSpeed(speed);
      m2.setSpeed(speed);
      m3.setSpeed(speed);
      m4.setSpeed(speed);
      m1.run(FORWARD);
      m3.run(FORWARD);
      m2.run(BACKWARD);
      m4.run(BACKWARD);
    }
  }

  delay(20);
}
