#include "Particle.h"
#include "message.h"
#include "client.h"

#include "Adafruit_DHT/Adafruit_DHT.h"

//SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

//DHT BEGIN
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

double temperature = 0.0;
double humidity = 0.0;

int internalLed = D7; // Instead of writing D7 over and over again, we'll write led2
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
//DHT END

#define WAIT_MESSAGE_DELAY 10
#define DO_WORK_PERIOD 60000
#define RECONNECTION_DELAY 60000

//https://docs.particle.io/reference/firmware/photon/#ipaddress
uint8_t server[] = { 192, 168, 178, 22 };
IPAddress IPfromBytes(server);
int portNumber = 11000;

//https://docs.particle.io/reference/firmware/photon/#tcpclient
char* DeviceName = "Photon"; 

TCPClient tcpClient; 
MCClient* mcClient;
Message* message = NULL; 

unsigned long lastTime;

void setup() {
    
    //DHT BEGIN
    pinMode(internalLed, OUTPUT);
    //DHT END
    
    //Make sure your Serial Terminal app is closed before powering your device
    Serial.begin(9600);
    // Now open your Serial Terminal, and hit any key to continue!
    //while(!Serial.available()) 
    //  Particle.process();

    WiFi.on(); 
    WiFi.connect();
    waitUntil(WiFi.ready);  
      
    mcClient = new MCClient(&tcpClient);  
      
    if (ConnectAndRegister())
    {
        //DHT BEGIN
        dht.begin();
        //DHT END   
        
        lastTime = millis();
    }
}

bool ConnectAndRegister(){
    if (Connect())
    {
        Register(); 
        return true;  
    }
    else
    {
        return false; 
    }
}
    
bool Connect() {
    Serial.println("connecting...");
    if (mcClient->Connect(IPfromBytes, portNumber))
    {
        Serial.println("connected");
        return true; 
    }
    else
    {
        Serial.println("connection failed");
        return false; 
    }
}

void Register() {
    if (mcClient->IsConnected())
    {
        //Serial.println("registering...");
        mcClient->Register(DeviceName); 
        mcClient->PublishData("*", "Sensor", "Temperature");
        mcClient->PublishData("*", "Sensor", "Humidity");
        mcClient->PublishData("*", "ToggleLed", "BoardLed");
        mcClient->PublishData("*", "Monitoring", "Reception"); 
        mcClient->SubscribeToCommand("*", "ToggleLed", "BoardLed");        
    }
}

void loop() {
    
    delay(WAIT_MESSAGE_DELAY);

    //We check for Message data filling the buffer
    int result = mcClient->BufferMessageData(); 
    if (result == MCClient::BUFFER_READY)
    {
        message = mcClient->Read();
        if (message != NULL) {
            HandleReceivedMessage();
            delete message;
            message = NULL;    
        }
    }

    //Periodically, we do some work
    if (millis() - lastTime > DO_WORK_PERIOD)
    {
        DoWork(); 
        lastTime = millis();
    }
            
    if (!mcClient->IsConnected())
    {
        Serial.println("disconnected");
        delay(RECONNECTION_DELAY);
        ConnectAndRegister(); 
    }
}

void HandleReceivedMessage() {
    //Serial.println("handling message");
    
    if (mcClient->IsConnected())
    {
        mcClient->SendData("*", "Monitoring", "Reception", "message received");
        //TODO
    }
}

void DoWork() {
    //Serial.println("do work");
    
    //DHT BEGIN 
    digitalWrite(internalLed, HIGH);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.getHumidity();
    // Read temperature as Celsius
    float t = dht.getTempCelcius();

    temperature = (double)t;
    humidity = (double)h;

    digitalWrite(internalLed, LOW);
    //DHT END
    
    mcClient->SendData("*", "Sensor", "Temperature", String(temperature,2));
    mcClient->SendData("*", "Sensor", "Humidity", String(humidity,2));
}

//https://docs.particle.io/reference/cli/#particle-serial-monitor
//https://docs.particle.io/guide/getting-started/modes/photon/#safe-mode