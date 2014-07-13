#define SERIAL_FIELDS_COUNT 3

#define SERIAL_COMMAND_FIELD 0
#define SERIAL_VALUE_FIELD 1
#define SERIAL_REQUEST_ID 2

#define STATUS_OK 1

#define E_SERIAL_INVALID_REQUEST -1
#define E_SERIAL_IDLE -2
#define E_SERIAL_INVALID_COMMAND -3
#define E_VOLTAGE_MEASURE_ERROR -4 
#define E_SERVO_POSIOTIONING_FAILURE -5
#define E_SERIAL_INVALID_RQ_ID -6

#define SC_HEALTHCHECK 0
#define SC_SERVO_POSITION 1
#define SC_MOTOR_SPEED 2


#define SERVO_VALUE_MIN 0
#define SERVO_VALUE_MAX 1024
#define SERVO_TARGET_OFFSET 10
#define SERVO_SLOW_SPEED 50
#define SERVO_MAX_DELTA 150

#define E_SERVO_WRONG_POSITION -10

#define MOTOR_VALUE_MIN 0
#define MOTOR_VALUE_NEUTRAL 50
#define MOTOR_VALUE_MAX 100

#define E_MOTOR_INVALID_SPEED -20

#define MAX_IDLE_TIME 1000


int requestID = 0;


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
float voltageValue = 0.0;
float VOLTAGE_CONST = 0.01563; // for 22KOhm/10KOhm
/* float VOLTAGE_CONST = 0.011842; // for 10KOhm/6.8KOhm */

float lastActionTime;

int setMotorSpeed(int speedTarget)
{
    int NEUTRAL = MOTOR_VALUE_NEUTRAL;
    motorSpeedTarget = speedTarget;

    if(speedTarget < MOTOR_VALUE_MIN || speedTarget > MOTOR_VALUE_MAX)
    {
        return E_MOTOR_INVALID_SPEED;
    }

    if(speedTarget < NEUTRAL)
    {
        //reverse
        speedTarget = (-speedTarget + NEUTRAL) * (255 /  NEUTRAL) + 5;

        digitalWrite(motorIn1A, HIGH);
        digitalWrite(motorIn2A, LOW);
        analogWrite(motorEnablePin, speedTarget);
    }
    else if(speedTarget > NEUTRAL)
    {
        //forward
        speedTarget = (speedTarget - NEUTRAL) * (255 /  NEUTRAL) + 5;

        digitalWrite(motorIn1A, LOW);
        digitalWrite(motorIn2A, HIGH);
        analogWrite(motorEnablePin, speedTarget);
    }
    else
    {
        digitalWrite(motorEnablePin, LOW);
    }

    return STATUS_OK;
}

int setServoPossition(int servoTarget)
{
    if(servoTarget < SERVO_VALUE_MIN || servoTarget > SERVO_VALUE_MAX)
    {
        return E_SERVO_WRONG_POSITION;
    }

    //setting new servo target
    servoPosTarget = servoTarget;

    return STATUS_OK;
}

int possitionServo()
{
    servoPosValue = analogRead(servoPosPin);

    if(servoPosTarget < servoPosValue - SERVO_TARGET_OFFSET)
    {
        digitalWrite(servoIn1A, HIGH);
        digitalWrite(servoIn2A, LOW);

        servoDelta = servoPosValue - servoPosTarget;
        if(servoDelta < SERVO_MAX_DELTA)
        {
            analogWrite(servoEnablePin, SERVO_SLOW_SPEED);
        }
        else
        {
            digitalWrite(servoEnablePin, HIGH);
        }
    }
    else if(servoPosTarget > servoPosValue + SERVO_TARGET_OFFSET)
    {
        digitalWrite(servoIn1A, LOW);
        digitalWrite(servoIn2A, HIGH);

        servoDelta =  servoPosTarget - servoPosValue;
        if(servoDelta < SERVO_MAX_DELTA)
        {
            analogWrite(servoEnablePin, SERVO_SLOW_SPEED);
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
    
    return STATUS_OK;
}


//voltage measurment
float measureVoltage()
{
    float v_sum = 0.0;

    for(int i=0; i < 10; i++)
    {
        voltageValue = VOLTAGE_CONST * analogRead(voltageInput);
        v_sum += voltageValue;
    }

    return v_sum / 10;
}

int resetSerial()
{
    for(int i=0; i < min(SERIAL_FIELDS_COUNT, fieldIndex+1); i++)
    {
        serial_values[i] = 0; // set the values to zero, ready for the next message
    }
    fieldIndex = 0;  // ready to start over

    return STATUS_OK;
}

int sendSerialResponse(int requestID, int status, String value)
{
    String serialOutput;
    
    serialOutput = String(requestID, DEC);// + ":" + String(status, DEC) + ":" + value;
    serialOutput += ":" + String(status, DEC) + ":" + value;

    Serial.println(serialOutput);

    return STATUS_OK;
}

int processSerial()
{
    int status = STATUS_OK;
    String result = "0";
    float value = 0.0;
    char buffer[10];
    String s_value;

    if( Serial.available())
    {
        char ch = Serial.read();

        if(ch >= '0' && ch <= '9') // is this an ascii digit between 0 and 9?
        {
            serial_values[fieldIndex] = (serial_values[fieldIndex] * 10) + (ch - '0'); 
        }
        else if (ch == ':')  // comma is our separator, so move on to the next field
        {
            fieldIndex++;   // increment field index 
        }
        else if (ch == '\r')
        {
            if(fieldIndex + 1 != SERIAL_FIELDS_COUNT)
            {
                status = E_SERIAL_INVALID_REQUEST;
                sendSerialResponse(-1, status, String(requestID, DEC));
                resetSerial();

                return status;
            }

            if(serial_values[SERIAL_REQUEST_ID] < 0 || serial_values[SERIAL_REQUEST_ID] == 0)
            {
                status = E_SERIAL_INVALID_RQ_ID;
                sendSerialResponse(-1, status, String(requestID, DEC));
                resetSerial();

                return status;
            }
            requestID = serial_values[SERIAL_REQUEST_ID];

            lastActionTime = millis();

            switch(serial_values[SERIAL_COMMAND_FIELD])
            {
                case SC_HEALTHCHECK:
                    value = measureVoltage();
                    if(value < 0)
                    {
                        status = (int) value;
                        break;
                    }
                    result = dtostrf(value, 4, 2, buffer);
                    result += "," + String(servoPosTarget, DEC) + "," + String(motorSpeedTarget, DEC);
                    break;

                case SC_SERVO_POSITION:
                    status = setServoPossition(serial_values[SERIAL_VALUE_FIELD]);
                    break;

                case SC_MOTOR_SPEED:
                    status = setMotorSpeed(serial_values[SERIAL_VALUE_FIELD]);
                    break;

                default:
                    //request error: invalid command
                    status = E_SERIAL_INVALID_COMMAND;
            }

            //send response
            if(status == STATUS_OK)
            {
                sendSerialResponse(requestID, status, result);
            }
            else
            {
                sendSerialResponse(requestID, status, result);
            }

            resetSerial();
        }
        else if (ch == '\n')
        {
            //do noting
        }
        else
        {
            status = E_SERIAL_INVALID_REQUEST;
            sendSerialResponse(-1, status, String(requestID, DEC));
            resetSerial();
            return status;
        }
    }

    return status;
}

void setup()
{
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

    lastActionTime = millis();
}

void loop()
{
    int status = STATUS_OK;

    if(lastActionTime < (millis() - MAX_IDLE_TIME))
    {
        //stop all motors
        setMotorSpeed(MOTOR_VALUE_NEUTRAL);
        lastActionTime = millis();

        //send error communicate
        sendSerialResponse(-1, E_SERIAL_IDLE, String(requestID, DEC));
    }
    //precaution against going back to zero the millis() result
    if(lastActionTime > millis()) lastActionTime = millis();

    status = processSerial();
    
    status = possitionServo();
    if(status != STATUS_OK)
    {
        sendSerialResponse(-1, E_SERVO_POSIOTIONING_FAILURE, String(requestID, DEC));
    }
}
