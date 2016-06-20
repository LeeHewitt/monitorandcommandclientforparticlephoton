#include "Particle.h"
#include "message.h"
#include "client.h"

//BEGIN DEMO CODE
#include "Adafruit_DHT/Adafruit_DHT.h"
//END

#define WAIT_MESSAGE_DELAY 50
#define DO_WORK_PERIOD 60000
#define RECONNECTION_DELAY 60000

SYSTEM_THREAD(ENABLED);

//https://docs.particle.io/reference/firmware/photon/#ipaddress
uint8_t server[] = { 192, 168, 178, 26 };
IPAddress IPfromBytes(server);
int portNumber = 11000;

//https://docs.particle.io/reference/firmware/photon/#tcpclient
TCPClient tcpClient; 

MCClient* mcClient;
char* DeviceName = "Photon A"; 
Message* message = NULL; 

unsigned long lastTime;

//BEGIN DEMO CODE
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

double temperature = 0.0;
double humidity = 0.0;

int externalGreenLED = D4;
bool externalGreenLEDIsOn = false;
int externalRedLED = D6;
bool externalRedLEDIsOn = false;
int boardLED = D7;
bool boardLEDIsOn = false;
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
//END

void setup() {
    
    //BEGIN DEMO CODE
    pinMode(boardLED, OUTPUT);
    pinMode(externalRedLED, OUTPUT);
    pinMode(externalGreenLED, OUTPUT);
    //END
    
    //Make sure your Serial Terminal app is closed before powering your device
    //Serial.begin(9600);
    //Now open your Serial Terminal, and hit any key to continue!
    //while(!Serial.available()) 
    //  Particle.process();

    //If SYSTEM_MODE is MANUAL
    WiFi.on(); 
    WiFi.connect();
    waitUntil(WiFi.ready);  
      
    mcClient = new MCClient(&tcpClient);  
     
    if (ConnectAndRegister()) {
        //BEGIN DEMO CODE 
        dht.begin();
        //END   
        
        lastTime = millis();
    }
}

//Connect to the Server, register the device and publish/subscribe to commands and data
bool ConnectAndRegister(){
    Serial.println("Connect");
    if (mcClient->Connect(IPfromBytes, portNumber)) {
        mcClient->Register(DeviceName); 
        
        //BEGIN DEMO CODE
        mcClient->PublishData("*", "Sensor", "Temperature");
        mcClient->PublishData("*", "Sensor", "Humidity");
        
        mcClient->PublishData("*", "BoardLED", "LEDStatus");
        mcClient->PublishData("*", "RedLED", "LEDStatus");
        mcClient->SubscribeToCommand("*", "ToggleLED", "RedLED");   
        mcClient->PublishData("*", "GreenLED", "LEDStatus");
        mcClient->SubscribeToCommand("*", "ToggleLED", "GreenLED");   
        //END 
        
        Serial.println("Registered");
        return true;  
    } else {
        return false; 
    }
}

//Listen to incoming messages and perform periodic work 
void loop() {
    if (mcClient->IsConnected()) {
        //We check for Message data filling the buffer
        int result = mcClient->ProcessTCPBuffer(); 
        if (result == MCClient::BUFFER_READY) {
            //When a message is available, we process it
            message = mcClient->Receive();
            if (message != NULL) {
                ProcessReceivedMessage();
                delete message;
                message = NULL;    
            }
        } else if (result == MCClient::BUFFER_NONE){
            delay(WAIT_MESSAGE_DELAY);    
        }
    
        //Periodically, we do some work (but if a message is currently received, we loop until we go it all)
        if ((result != MCClient::BUFFER_PARTIAL) && (millis() - lastTime > DO_WORK_PERIOD)) {
            DoWork(); 
            lastTime = millis();
        }  
    }
    else {
        //If the client is disconnected, reconnect and register
        delay(RECONNECTION_DELAY);
        ConnectAndRegister();       
    }
}

//Process received message
void ProcessReceivedMessage() {
    Serial.println("Process message");
    if (mcClient->IsConnected()) {
        //BEGIN DEMO CODE
        if (message->Parameter == "ToggleLED") {
            if (message->Name == "GreenLED") {
                //BEGIN your functionality
                externalGreenLEDIsOn = !externalGreenLEDIsOn;
                digitalWrite(externalGreenLED, externalGreenLEDIsOn ? HIGH : LOW);
                mcClient->SendData("*", "GreenLED", "LEDStatus", externalGreenLEDIsOn ? "On" : "Off");
                //END      
            } else if (message->Name == "RedLED") {
                //BEGIN your functionality
                externalRedLEDIsOn = !externalRedLEDIsOn;
                digitalWrite(externalRedLED, externalRedLEDIsOn ? HIGH : LOW);
                mcClient->SendData("*", "RedLED", "LEDStatus", externalRedLEDIsOn ? "On" : "Off");
                    
            }
        }
        //END
    }
}

void DoWork() {
    Serial.println("Do work");
    
    //BEGIN DEMO CODE
    digitalWrite(boardLED, HIGH);
    mcClient->SendData("*", "BoardLED", "LEDStatus", "On");

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.getHumidity();
    // Read temperature as Celsius
    float t = dht.getTempCelcius();

    temperature = (double)t;
    humidity = (double)h;

    mcClient->SendData("*", "Sensor", "Temperature", String(temperature,1));
    mcClient->SendData("*", "Sensor", "Humidity", String(humidity,1));

    digitalWrite(boardLED, LOW);
    mcClient->SendData("*", "BoardLED", "LEDStatus", "Off");
    //END
}

//https://docs.particle.io/reference/cli/#particle-serial-monitor
//https://docs.particle.io/guide/getting-started/modes/photon/#safe-mode