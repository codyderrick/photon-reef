// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"

// This #include statement was automatically added by the Particle IDE.
#include "crc8.h"

// This #include statement was automatically added by the Particle IDE.
#include "ds18x20.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

#include "TimeAlarms/TimeAlarms.h"

uint8_t sensors[80];
char temperature[10];
char relays[265];

void log(char* msg)
{
    Particle.publish("log", msg);
    delay(100);
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
       { 23,15 },
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
    Particle.function("relay", relayControl);
    Particle.function("relays", relayControl);
        
    Particle.subscribe("temperature_alert", handleTemperatureAlert);
    
    // register spark variables
    Particle.variable("temperature", &temperature, STRING);
    // Spark.variable("ph", &ph_data, STRING);
    Particle.variable("relays", &relays, STRING);

    // Lets listen for the hook response
    Particle.subscribe("hook-response/timezone", gotTimeZone, MY_DEVICES);
    delay(1000);
    
    // publish the event that will trigger our Webhook
    Particle.publish("timezone");
    delay(10000);
    
    updateRelayJson();
    
}

void loop() {
    getTemperature();
    Alarm.delay(1000);
    delay(60000);
}

void gotTimeZone(const char *name, const char *data) {
    
    log("parsing timezone");
    
    String str = String(data);
    str.replace("\r", "");
    
    unsigned int length = str.length();
    char json[length];
    str.toCharArray(json, length);
    
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);
    
    if (!root.success()) {
        log("could not update timezone, using hard coded");
        Time.zone(-6);
    }
    else{
        int dstOffset = root["dstOffset"];
        int rawOffset = root["rawOffset"];
        int tzOffset = (dstOffset + rawOffset) / 3600;
        char msg[100];
        sprintf(msg, "updated timezone using offset %i", tzOffset);
        log(msg);
        Time.zone(tzOffset);
    }
    
    // don't start alarms until after getting tz offset
    // main lights
    Alarm.alarmRepeat(schedules[0].on[0],schedules[0].on[1],0, turnOnMainLights);
    Alarm.alarmRepeat(schedules[0].off[0],schedules[0].off[1],0, turnOffMainLights);
    
    // sump(scrubber)
    Alarm.alarmRepeat(schedules[1].on[0],schedules[1].on[1],0, turnOnSumpLights);
    Alarm.alarmRepeat(schedules[1].off[0],schedules[1].off[1],0, turnOffSumpLights);
    
    Alarm.timerRepeat(1,0,0, saveProbes);
}

void turnOnMainLights(){
    log("turning on main lights");
    relayControl("r1,ON");
    updateRelayJson();
}

void turnOffMainLights(){
    log("turning off main lights");
    relayControl("r1,OFF");
    updateRelayJson();
}

void turnOnSumpLights(){
    log("turning on sump lights");
    relayControl("r2,ON");
    updateRelayJson();
}

void turnOffSumpLights(){
    log("turning off sump lights");
    relayControl("r2,OFF");
    updateRelayJson();
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
    
    updateRelayJson();
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

    for (uint8_t i=0; i<numsensors; i++)
    {
        if (sensors[i*OW_ROMCODE_SIZE+0] == 0x10 || sensors[i*OW_ROMCODE_SIZE+0] == 0x28) //0x10=DS18S20, 0x28=DS18B20
        {
            //log("Found a DS18B20");
			if ( DS18X20_read_meas( &sensors[i*OW_ROMCODE_SIZE], &subzero, &cel, &cel_frac_bits) == DS18X20_OK ) {
				char sign = (subzero) ? '-' : '+';
				int frac = cel_frac_bits*DS18X20_FRACCONV;
				//char str_f[10];
				float c_temp = (frac * .0001f) + cel;
                float f_temp = (c_temp * 1.8 + 32);
				int t_temp = f_temp*10;
				sprintf(temperature, "%i", t_temp);
                //sprintf(str_f, "%.2f", f_temp);
				Spark.publish("temp-float", String(f_temp), 60, PRIVATE);
				Spark.publish("tankTemperature", temperature, 60, PRIVATE);
				if(f_temp>83){
					Spark.publish("temperature_alert", "HIGH", 60, PRIVATE);
				}
                if(f_temp<75){
					Spark.publish("temperature_alert", "LOW", 60, PRIVATE);
				}
			}
			else
			{
			    Spark.publish("log", "CRC Error (lost connection?)");
			}
        }
    }
    log("finished measurement");
    //Alarm.timerOnce(1200, getTemperature);
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

void updateRelayJson(){
    StaticJsonBuffer<256> jsonBuffer;

    // {id: 'r1', name: 'Kessil a350w', state: 'auto', scheduled: true}
    JsonArray &root = jsonBuffer.createArray();
    JsonObject &relay = jsonBuffer.createObject();
    relay["id"] = RELAY1;
    relay["scheduled"] = true;
    relay["state"] = digitalRead(RELAY1);
    root.add(relay);
    
    relay["id"] = RELAY2;
    relay["scheduled"] = true;
    relay["state"] = digitalRead(RELAY2);
    root.add(relay);
    
    relay["id"] = RELAY3;
    relay["state"] = digitalRead(RELAY3);
    root.add(relay);
    
    relay["id"] = RELAY4;
    relay["state"] = digitalRead(RELAY4);
    root.add(relay);
    
    root.prettyPrintTo(relays, sizeof(relays));
}

void saveProbes(){
    log("saving probes");
    char msg[100];
    sprintf(msg, "{ \"collection\": \"probes\", \"temperature\": \"%s\" }", temperature);
    Spark.publish("save-probes", msg, 60, PRIVATE);
}