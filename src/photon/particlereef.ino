// This #include statement was automatically added by the Particle IDE.
#include "crc8.h"

// This #include statement was automatically added by the Particle IDE.
#include "ds18x20.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

int hour;

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

// char temperature[10];
// char ph_data[10];//we make a 20 byte character array to hold incoming data from the pH stamp.

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
    log("turn relays off");    
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
    
    //register the Spark function
    Spark.function("relay", relayControl);
    
    // register spark variables
    Spark.variable("temperature", &temperature, STRING);
    // Spark.variable("ph", &ph_data, STRING);
    
    Time.zone(-6);
}

void loop() {



    log("get temperature");
    getTemperature();
    // // calibrate(temp_c);
    // // getPh();
    // //int t_temp = random(720, 820);
    // int t_ph = random(70, 90);
    
    // float f_temp = ((float)t_temp) * 0.1f;
    // float f_ph = ((float)t_ph) * 0.1f;
    
    // // sprintf(buf, "%.1f", myFloat)
    // sprintf(temperature, "%i", t_temp);
    // sprintf(ph_data, "%i", t_ph);
    
    // // dtostrf(f_temp, 4, 2, temperature);
    
    // Spark.publish("tankTemperature", temperature, 60, PRIVATE);
    // Spark.publish("tankPh", ph_data, 60, PRIVATE);
    
    //loopRelays();
    
    delay(5000);
    
    log("get current time");
    hour = Time.hour();
    hour = hour + 1;
    
    log("set lights");
    setMainLights(hour);
    
    delay(2000);
    log("set sump lights");
    setSumpLights(hour);
    
    delay(60000);
}

void setMainLights(int hour){
    char msg[255];
    sprintf(msg, "main lights current hour:%i", hour); //rtc.hour(currentTime));
    log(msg);
    
    if (hour >= 14 && hour < 21){
        int t = relayControl("r1,ON");
    }
    else{
        int t = relayControl("r1,OFF");
    }
}

void setSumpLights(int hour){
    char msg[255];
    sprintf(msg, "scrubber current hour:%i", hour); //rtc.hour(currentTime));
    log(msg);
    
    // bool off = (hour > 11 && hour < 23);
    // sprintf(msg, "scrubber check:%d", off);
    // log(msg);
    if (hour > 11 && hour < 23){
        log("turning scrubber lights off");
        int t = relayControl("r2,OFF");
    }
    else {
        log("turning scrubber lights on");
        int t = relayControl("r2,ON");
    }
}

void loopRelays(){
    log("turn relays on");    
    relayControl("r1,ON");
    relayControl("r2,ON");
    relayControl("r3,ON");
    relayControl("r4,ON");
    delay(10000);
    log("turn relays off");
    relayControl("r1,OFF");
    relayControl("r2,OFF");
    relayControl("r3,OFF");
    relayControl("r4,OFF");
}

// command format r1,ON
int relayControl(String command) {
    int relayState = 0;
    // parse the relay number
    int relayNumber = command.charAt(1) - '0';
    // do a sanity check
    if (relayNumber < 1 || relayNumber > 4) return -1;
    
    // find out the state of the relay
    if (command.substring(3,6) == "OFF") relayState = 1;
    else if (command.substring(3,5) == "ON") relayState = 0;
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
    sprintf(msg, "Found %i sensors", numsensors);
    log(msg);

    
    for (uint8_t i=0; i<numsensors; i++)
    {
        if (sensors[i*OW_ROMCODE_SIZE+0] == 0x10 || sensors[i*OW_ROMCODE_SIZE+0] == 0x28) //0x10=DS18S20, 0x28=DS18B20
        {
            //log("Found a DS18B20");
			if ( DS18X20_read_meas( &sensors[i*OW_ROMCODE_SIZE], &subzero, &cel, &cel_frac_bits) == DS18X20_OK ) {
				char sign = (subzero) ? '-' : '+';
				int frac = cel_frac_bits*DS18X20_FRACCONV;
				sprintf(msg, "Sensor# %d (%02X%02X%02X%02X%02X%02X%02X%02X) =  : %c%d.%04d\r\n",i+1,
				sensors[(i*OW_ROMCODE_SIZE)+0],
				sensors[(i*OW_ROMCODE_SIZE)+1],
				sensors[(i*OW_ROMCODE_SIZE)+2],
				sensors[(i*OW_ROMCODE_SIZE)+3],
				sensors[(i*OW_ROMCODE_SIZE)+4],
				sensors[(i*OW_ROMCODE_SIZE)+5],
				sensors[(i*OW_ROMCODE_SIZE)+6],
				sensors[(i*OW_ROMCODE_SIZE)+7],
				sign,
				cel,
				frac
				);
				log(msg);
				
				char str_c[10];
				sprintf(str_c, "%c%d.%04d", sign, cel, frac);
	
				float c_temp = (frac * .0001f) + cel;//* 10000;
				int t_temp = (c_temp * 1.8 + 32)*10;
				sprintf(temperature, "%i", t_temp);
				Spark.publish("temp-celcius", str_c, 60, PRIVATE);
				Spark.publish("tankTemperature", temperature, 60, PRIVATE);
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
