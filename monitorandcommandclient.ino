#include "Particle.h"
#include "message.h"
#include "client.h"

#include "Adafruit_DHT/Adafruit_DHT.h"

#define WAIT_MESSAGE_DELAY 0
#define DO_WORK_PERIOD 60000
#define RECONNECTION_DELAY 60000

//SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

//https://docs.particle.io/reference/firmware/photon/#ipaddress
uint8_t server[] = { 192, 168, 178, 22 };
IPAddress IPfromBytes(server);
int portNumber = 11000;

//https://docs.particle.io/reference/firmware/photon/#tcpclient
TCPClient tcpClient; 

MCClient* mcClient;
char* DeviceName = "Photon"; 
Message* message = NULL; 

unsigned long lastTime;

//DHT22 BEGIN
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

double temperature = 0.0;
double humidity = 0.0;

int internalLED = D7; // Instead of writing D7 over and over again, we'll write led2
bool internalLEDIsOn = false;
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
//DHT END

void setup() {
    
    //DHT22 BEGIN
    pinMode(internalLED, OUTPUT);
    //DHT22 END
    
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
     
    if (ConnectAndRegister())
    {
        //DHT22 BEGIN
        dht.begin();
        //DHT22 END   
        
        lastTime = millis();
    }
}

//Connect to the Server, register the device and publish/subscribe to commands and data
bool ConnectAndRegister(){
    Serial.println("Connect");
    if (mcClient->Connect(IPfromBytes, portNumber))
    {
        mcClient->Register(DeviceName); 
        mcClient->PublishData("*", "Sensor", "Temperature");
        mcClient->PublishData("*", "Sensor", "Humidity");
        mcClient->PublishData("*", "BoardLED", "LEDStatus");
        mcClient->PublishData("*", "Monitoring", "Reception"); 
        mcClient->SubscribeToCommand("*", "ToggleLED", "BoardLED");   
        Serial.println("Registered");
        return true;  
    }
    else
    {
        return false; 
    }
}

//Listen to incoming messages and perform periodic work 
void loop() {
    
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

    //Periodically, we do some work (but if a message is currently received, we loop until it is complete)
    if (result != MCClient::BUFFER_PARTIAL && millis() - lastTime > DO_WORK_PERIOD)
    {
        DoWork(); 
        lastTime = millis();
    }
     
    //If the client is disconnected, reconnect and register
    if (!mcClient->IsConnected())
    {
        delay(RECONNECTION_DELAY);
        ConnectAndRegister(); 
    }
}

//Process received message
void ProcessReceivedMessage() {
    Serial.println("Process message");
    if (mcClient->IsConnected())
    {
        internalLEDIsOn = !internalLEDIsOn;
        digitalWrite(internalLED, internalLEDIsOn ? HIGH : LOW);
            
        mcClient->SendData("*", "BoardLED", "LEDStatus", internalLEDIsOn ? "On" : "Off");
    }
}

void DoWork() {
    Serial.println("Do work");
    
    //DHT BEGIN 
    digitalWrite(internalLED, HIGH);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.getHumidity();
    // Read temperature as Celsius
    float t = dht.getTempCelcius();

    temperature = (double)t;
    humidity = (double)h;

    digitalWrite(internalLED, LOW);
    //DHT END
    
    mcClient->SendData("*", "Sensor", "Temperature", String(temperature,2));
    mcClient->SendData("*", "Sensor", "Humidity", String(humidity,2));
}

//https://docs.particle.io/reference/cli/#particle-serial-monitor
//https://docs.particle.io/guide/getting-started/modes/photon/#safe-mode