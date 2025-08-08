#include <SPI.h>
#include <mcp_can.h>

/* Please modify SPI_CS_PIN to adapt to different baords.

   CANBed V1        - 17
   CANBed M0        - 3
   CAN Bus Shield   - 9
   CANBed 2040      - 9
   CANBed Dual      - 9
   OBD-2G Dev Kit   - 9
   Hud Dev Kit      - 9
*/

#define SPI_CS_PIN  9

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_PRM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05

#define CAN_ID_PID          0x7DF

void set_mask_filt()
{
    // set mask, set both the mask to 0x3ff

    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    // set filter, we can receive id from 0x04 ~ 0x09

    CAN.init_Filt(0, 0, 0x7E8);                 
    CAN.init_Filt(1, 0, 0x7E8);

    CAN.init_Filt(2, 0, 0x7E8);
    CAN.init_Filt(3, 0, 0x7E8);
    CAN.init_Filt(4, 0, 0x7E8); 
    CAN.init_Filt(5, 0, 0x7E8);
}

void sendPid(unsigned char __pid) {
    unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};
    CAN.sendMsgBuf(CAN_ID_PID, 0, 8, tmp);
}

bool getSpeed(int *s)
{
    sendPid(PID_VEHICLE_SPEED);
    unsigned long __timeout = millis();

    while (millis() - __timeout < 1000) {
        unsigned char len = 0;
        unsigned char buf[8];
        unsigned long canId;

        if (CAN_MSGAVAIL == CAN.checkReceive()) {
            CAN.readMsgBuf(&canId, &len, buf);  // FIXED: use 3 arguments

            if (buf[1] == 0x41) {
                *s = buf[3];
                return true;
            }
        }
    }

    return false;
}

const int pinPwrCtrl = 12;          // for RP2040 verison
//const int pinPwrCtrl = A3;        // for Atmaega32U4 version


void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(pinPwrCtrl, OUTPUT);
    digitalWrite(pinPwrCtrl, HIGH);

    while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ)) {  // FIXED
        Serial.println("CAN init fail, retry...");
        delay(100);
    }
    CAN.setMode(MCP_NORMAL);
    Serial.println("CAN init ok!");
    set_mask_filt();
}

void loop() {
    // Serial.println("start loop function");

    int __speed = 0;
    int ret = getSpeed(&__speed);
    // Serial.println(ret);
    if(ret)
    {
        Serial.print("Vehicle Speed: ");
        Serial.print(__speed);
        Serial.println(" kmh");
    }
    delay(500);
}

// END FILE
