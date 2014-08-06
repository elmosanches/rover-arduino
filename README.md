# Philosophy
Communication with arduino is realised by SerialPort. A request formated according to the specifiaction is sent to arudino and arduino sends back a response with status of the device.

## REQUEST:

A request consists of 3 fields separated with ':'

\<COMMAND\>:\<VALUE\>:\<RQ-ID\>

Available commands:

00 - healthcheck
    value: 0 - check voltage
    value: 1 - stop motors

    result: \<vlotage\>,\<servo_target\>,\<motor_speed_target\>

01 - servo position target
    value: 0 - 1024

    result: 0

02 - motor speed target
    value: 0 - 100

    result: 0

RQ-ID: identyfication number of the request. Arduino will use that ID in the response to indicate which requst the response is to.

## RESPONSE:

\<RQ-ID\>:\<STATUS\>:\<RESULT\>

RQ-ID: Request ID to which the response is prepared. If the response is an internal schema error communicate (idle error for example) RQ-ID is -1

Status:

1: OK

-1: Invalid request

-2: idle timeout - operating suspended (due to no request withing required time - usualy 1s)

-3: Invalid command

-4: Voltage measurement error

-5: Positioning servo feilure

-6: Invalid request id

-7: Voltage too low - operating suspended

-10: invalid servo position target

-20: invalid motor speed target
