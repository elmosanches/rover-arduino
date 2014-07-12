#define SERIAL_FIELDS_COUNT 3
#define SERVO_MAX_DELTA 150

#define SERIAL_COMMAND_FIELD 0
#define SERIAL_VALUE_FIELD 1
#define SERIAL_REQUEST_ID 2

#define SC_HEALTHCHECK 0
#define SC_SERVO_POSITION 1
#define SC_MOTOR_SPEED 2

#define E_SERIAL_INVALID_REQUEST = -1
#define E_SERVO_WRONG_POSITION = -10


int SERVO_VALUE_MIN = 0;
int SERVO_VALUE_MAX = 1024;

int MOTOR_VALUE_MIN = 0;
int MOTOR_VALUE_NEUTRAL = 10;
int MOTOR_VALUE_MAX = 20;

int fieldIndex = 0;               // the current field being received
int serial_values[SERIAL_FIELDS_COUNT];  // array holding values for all the fields

int servoPosPin = A0;
int servoPosValue = 0;

int servoPosTarget = 0;

int servoEnablePin = 10;
int servoIn1A = 4;
int servoIn2A = 5;
int servoDelta = 0;

int motorSpeedTarget = 10;

int motorEnablePin = 11;
int motorIn1A = 6;
int motorIn2A = 7;

int voltageInput = A2;
double voltageValue = 0.0;
/* double VOLTAGE_CONST = 0.01563; // for 22KOhm/10KOhm */
double VOLTAGE_CONST = 0.011842; // for 10KOhm/6.8KOhm


int setMotorSpeed(int speedTarget)
{
  int NEUTRAL = MOTOR_VALUE_NEUTRAL;
  //MOTOR
  if(speedTarget < NEUTRAL)
  {
    //reverse
    speedTarget = (-speedTarget + NEUTRAL) * (255 /  NEUTRAL) + 5;
    Serial.print("new reverse speed target ready: ");
    Serial.println(speedTarget);
    
    digitalWrite(motorIn1A, HIGH);
    digitalWrite(motorIn2A, LOW);
    /* digitalWrite(motorEnablePin, HIGH); */
    analogWrite(motorEnablePin, speedTarget);
  }
  else if(speedTarget > NEUTRAL)
  {
    //forward
    speedTarget = (speedTarget - NEUTRAL) * (255 /  NEUTRAL) + 5;
    Serial.print("new forward speed target ready: ");
    Serial.println(speedTarget);
    
    digitalWrite(motorIn1A, LOW);
    digitalWrite(motorIn2A, HIGH);
    /* digitalWrite(motorEnablePin, HIGH); */
    analogWrite(motorEnablePin, speedTarget);
  }
  else digitalWrite(motorEnablePin, LOW);
 
  return 1; 
}

int setServoPossition(int servoTarget)
{
    int status = 1;

    //setting new servo target
    if(servoTarget >= SERVO_VALUE_MIN and servoTarget <= SERVO_VALUE_MAX)
    {
        servoPosTarget = servoTarget;
    }
    else
    {
        // invalid servo target value
        status = -1;
    }

    return status; 
}

int possitionServo()
{
    //SERVO
    servoPosValue = analogRead(servoPosPin);

    if(servoPosTarget < servoPosValue - 10)
    {

        /* Serial.print("pos too high: "); */
        /* Serial.println(servoPosValue); */

        digitalWrite(servoIn1A, HIGH);
        digitalWrite(servoIn2A, LOW);

        servoDelta = servoPosValue - servoPosTarget;
        if(servoDelta < SERVO_MAX_DELTA)
        {
            analogWrite(servoEnablePin, 50);
        }
        else
        {
            digitalWrite(servoEnablePin, HIGH);
        }
    }
    else if(servoPosTarget > servoPosValue + 10)
    {
        /* Serial.print("pos too low: "); */
        /* Serial.println(servoPosValue); */

        digitalWrite(servoIn1A, LOW);
        digitalWrite(servoIn2A, HIGH);

        servoDelta =  servoPosTarget - servoPosValue;
        if(servoDelta < SERVO_MAX_DELTA)
        {
            analogWrite(servoEnablePin, 50);
        }
        else
        {
            digitalWrite(servoEnablePin, HIGH);
        }
    }
    else
    {
        digitalWrite(servoEnablePin, LOW);
    }
}


//voltage measurment
double measureVoltage()
{
    double v_sum = 0.0;

    for(int i=0; i < 10; i++)
    {
        voltageValue = VOLTAGE_CONST * analogRead(voltageInput);
        v_sum += voltageValue;
    }
    return v_sum / 10;
}


void setup(){
  Serial.begin(9600);

  pinMode(servoEnablePin, OUTPUT);
  pinMode(servoIn1A, OUTPUT);
  pinMode(servoIn2A, OUTPUT);
  digitalWrite(servoEnablePin, LOW);
  
  pinMode(motorEnablePin, OUTPUT);
  pinMode(motorIn1A, OUTPUT);
  pinMode(motorIn2A, OUTPUT);
  digitalWrite(motorEnablePin, LOW);
  
  setMotorSpeed(MOTOR_VALUE_NEUTRAL);
  
  servoPosTarget = SERVO_VALUE_MAX / 2;
}

void loop()
{
  if( Serial.available())
  {
    char ch = Serial.read();

    if(ch >= '0' && ch <= '9') // is this an ascii digit between 0 and 9?
    {
      if(fieldIndex < SERIAL_FIELDS_COUNT) {
        serial_values[fieldIndex] = (serial_values[fieldIndex] * 10) + (ch - '0'); 
      }
    }
    else if (ch == ':')  // comma is our separator, so move on to the next field
    {
      fieldIndex++;   // increment field index 
    }
    else if (ch == '\r')
    {
        //if fildIndex < SERIAL_FIELDS_COUNT error invalid request

        switch(serial_values[SERIAL_COMMAND_FIELD])
        {
            case SC_HEALTHCHECK:
                break;
            case SC_SERVO_POSITION:
                break;
            case SC_MOTOR_SPEED:
                break;
            /* default: */
            /*     //request error: invalid command */

        }


      /* //setting new servo target */
      /* if(serial_values[SERVO_SERIAL_FIELD] >= 0 and serial_values[SERVO_SERIAL_FIELD] <= 1024) */
      /* { */
      /*   servoPosTarget = serial_values[SERVO_SERIAL_FIELD]; */
      /*   Serial.print("OK,"); */
      /* } */
      /* else */
      /* { */
      /*   // invalid servo target value */
      /*   Serial.print("E2,"); */
      /* } */

      /* if(serial_values[MOTOR_SERIAL_FIELD] >= MOTOR_VALUE_MIN && serial_values[MOTOR_SERIAL_FIELD] <= MOTOR_VALUE_MAX) */
      /* { */
      /*   setMotorSpeed(serial_values[MOTOR_SERIAL_FIELD]); */
      /*   Serial.print("OK"); */
      /* } */
      /* else */
      /* { */
      /*   // invalid motor speed target value */
      /*   Serial.print("E3"); */
      /* } */
      /* Serial.println(""); */


      /* for(int i=0; i < min(SERIAL_FIELDS_COUNT, fieldIndex+1); i++) */
      /* { */
      /*   serial_values[i] = 0; // set the values to zero, ready for the next message */
      /* } */
      /* fieldIndex = 0;  // ready to start over */
    }
    else
    {
        //error
    }
  }
}

