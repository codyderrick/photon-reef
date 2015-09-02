// This #include statement was automatically added by the Particle IDE.
#include "crc8.h"

// This #include statement was automatically added by the Particle IDE.
#include "ds18x20.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

#include "TimeAlarms/TimeAlarms.h"

uint8_t sensors[80];
char temperature[10];

void log(char* msg)
{
    Spark.publish("log", msg);
    //delay(500);
}

int RELAY1 = D1;
int RELAY2 = D2;
int RELAY3 = D3;
int RELAY4 = D4;

struct schedule {
    int relay;
    int on[2];
    int off[2];
};

schedule schedules[2] {
    {
       RELAY1,
       { 14,0 },
       { 21,30 }
    },
    {
       RELAY2,
       { 23,0 },
       { 12,0 }
    } };

// float ph = 0;//used to store the results of converting the char to float for later use. 

void setup() {
    // initialize DS18x20
    ow_setPin(D0);
    
    //Initilize the relay control pins as output
    log("initializing relays");
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    
    // Initialize all relays to an OFF state
    log("turn relays off/on");    
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
    
    //register the Spark function
    Spark.function("relay", relayControl);
    Spark.function("relayStates", getRelayStates);
        
    Spark.subscribe("temperature_alert", handleTemperatureAlert);
    
    // register spark variables
    Spark.variable("temperature", &temperature, STRING);
    // Spark.variable("ph", &ph_data, STRING);

    
    Time.zone(-6);
    
    // main lights
    Alarm.alarmRepeat(schedules[0].on[0],schedules[0].on[1],0, turnOnMainLights);
    Alarm.alarmRepeat(schedules[0].off[0],schedules[0].off[1],0, turnOffMainLights);
    
    // sump(scrubber)
    Alarm.alarmRepeat(23,0,0, turnOnSumpLights);
    Alarm.alarmRepeat(12,0,0, turnOffSumpLights);
    
    Alarm.timerRepeat(300, getTemperature); 
    
}

void loop() {
    Alarm.delay(1000);
}

char* getRelayStates(String command){
    
}

// command format r1,ON; r2,OFF; r1,AUTO
int relayControl(String command) {
    int relayState = 0;
    // parse the relay number
    int relayNumber = command.charAt(1) - '0';
    // do a sanity check
    if (relayNumber < 1 || relayNumber > 4) return -1;
    
    
    // find out the intended state of the relay
    if (command.substring(3,6) == "OFF") relayState = 1;
    else if (command.substring(3,5) == "ON") relayState = 0;
    else if (command.substring(3,7) == "AUTO"){
        for(int i = 0; i < sizeof(schedules); i++){
            relayState = 1;
            if(relayNumber == schedules[i].relay){
                if((Time.hour() > schedules[i].on[0] && Time.minute() > schedules[i].on[1]) 
                    && (Time.hour() < schedules[i].off[0] && Time.minute() < schedules[i].off[1])) relayState = 0;
            }
        }
    }
    else return -1;
    
    // write to the appropriate relay
    char msg[15];
    sprintf(msg, "relay: %i-state:%i", relayNumber, relayState);
    log(msg);
    
    digitalWrite(relayNumber, relayState);
    return 1;
}

// void calibrate(float temp_f)
// {
//     Serial1.print(temp_f); //calibrate with current temp
//     Serial1.write(0x0D);//carriage return
//     delay(10);
// }

void getTemperature()
{
    uint8_t subzero, cel, cel_frac_bits;
    char msg[100];
    log("Starting measurement");    
    
    DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL ); //Asks all DS18x20 devices to start temperature measurement, takes up to 750ms at max resolution
    delay(1000); //If your code has other tasks, you can store the timestamp instead and return when a second has passed.

    uint8_t numsensors = ow_search_sensors(10, sensors);
    // sprintf(msg, "Found %i sensors", numsensors);
    // log(msg);

    for (uint8_t i=0; i<numsensors; i++)
    {
        if (sensors[i*OW_ROMCODE_SIZE+0] == 0x10 || sensors[i*OW_ROMCODE_SIZE+0] == 0x28) //0x10=DS18S20, 0x28=DS18B20
        {
            //log("Found a DS18B20");
			if ( DS18X20_read_meas( &sensors[i*OW_ROMCODE_SIZE], &subzero, &cel, &cel_frac_bits) == DS18X20_OK ) {
				char sign = (subzero) ? '-' : '+';
				int frac = cel_frac_bits*DS18X20_FRACCONV;
				// sprintf(msg, "Sensor# %d (%02X%02X%02X%02X%02X%02X%02X%02X) =  : %c%d.%04d\r\n",i+1,
				// sensors[(i*OW_ROMCODE_SIZE)+0],
				// sensors[(i*OW_ROMCODE_SIZE)+1],
				// sensors[(i*OW_ROMCODE_SIZE)+2],
				// sensors[(i*OW_ROMCODE_SIZE)+3],
				// sensors[(i*OW_ROMCODE_SIZE)+4],
				// sensors[(i*OW_ROMCODE_SIZE)+5],
				// sensors[(i*OW_ROMCODE_SIZE)+6],
				// sensors[(i*OW_ROMCODE_SIZE)+7],
				// sign,
				// cel,
				// frac
				// );
				// log(msg);
				
				char str_c[10];
				sprintf(str_c, "%c%d.%04d", sign, cel, frac);
	
				float c_temp = (frac * .0001f) + cel;//* 10000;
				int t_temp = (c_temp * 1.8 + 32)*10;
				sprintf(temperature, "%i", t_temp);
				//Spark.publish("temp-celcius", str_c, 60, PRIVATE);
				Spark.publish("tankTemperature", temperature, 60, PRIVATE);
				if(t_temp>810){
					Spark.publish("temperature_alert", "HIGH", 60, PRIVATE);
				}
			}
			else
			{
			    Spark.publish("log", "CRC Error (lost connection?)");
			}
        }
    }
    log("finished measurement");
}

// void getPh()
// {
//     Serial1.print("R\r");
//     delay(10);
//     if(Serial1.available() > 0)
//     {
//         Serial1.readBytesUntil(13,ph_data,20);
//         ph = atof(ph_data);
//     }
// }

void handleTemperatureAlert(const char *event, const char *data){
	if (strcmp(data,"HIGH")==0) {
    		relayControl("r3,OFF");
	}
}
