#include "Particle.h"
#include "message.h"
#include "client.h"

//BEGIN DEMO CODE
#include "Adafruit_DHT/Adafruit_DHT.h"
//END

#define WAIT_MESSAGE_DELAY_INCREMENT 10
#define MAXIMUM_WAIT_MESSAGE_DELAY 1000
#define DO_WORK_PERIOD 60000
#define RECONNECTION_DELAY 60000

//Uncomment to disable cloud functionnalities
//SYSTEM_MODE(MANUAL);

SYSTEM_THREAD(ENABLED);

//https://docs.particle.io/reference/firmware/photon/#ipaddress
uint8_t server[] = { 192, 168, 178, 22 };
IPAddress IPfromBytes(server);
int portNumber = 11000;

//https://docs.particle.io/reference/firmware/photon/#tcpclient
TCPClient tcpClient; 

MCClient* mcClient;
char* DeviceName = "Photon A"; 
Message* message = NULL; 

unsigned long lastTime;

int currentWaitMessageDelay = 0;

//BEGIN DHT22 DEMO CODE
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

int rgbRed = 0;
int rgbGreen = 0;
int rgbBlue = 0;

int boardLED = D7;
bool boardLEDIsOn = false;
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
//END

void setup() {

    //Make sure your Serial Terminal app is closed before powering your device
    //Serial.begin(9600);
    //Now open your Serial Terminal, and hit any key to continue!
    //while(!Serial.available()) 
    //  Particle.process();

    //If SYSTEM_MODE is MANUAL
    //WiFi.on(); 
    //WiFi.connect();
    //waitUntil(WiFi.ready);  

    //Turn down Status LED if needed 
    RGB.control(true); 
    RGB.brightness(255);
    RGB.color(rgbRed, rgbGreen, rgbBlue); 

    //BEGIN DHT22 DEMO CODE
    pinMode(boardLED, OUTPUT);
    pinMode(externalRedLED, OUTPUT);
    pinMode(externalGreenLED, OUTPUT);
    //END
      
    mcClient = new MCClient(&tcpClient);  
     
    if (ConnectAndRegister()) {
        //BEGIN DHT22 DEMO CODE
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
        
        //BEGIN DHT22 DEMO CODE
        mcClient->PublishData("*", "Sensor", "Temperature");
        mcClient->PublishData("*", "Sensor", "Humidity");
        
        mcClient->PublishData("*", "BoardLED", "LEDStatus");
        
        mcClient->PublishData("*", "RedLED", "LEDStatus");
        mcClient->SubscribeToCommand("*", "RedLED", "ToggleLED");   
        mcClient->PublishData("*", "GreenLED", "LEDStatus");
        mcClient->SubscribeToCommand("*", "GreenLED", "ToggleLED");
        
        mcClient->PublishData("*", "RGBLED", "RGBRed");
        mcClient->SubscribeToCommand("*", "RGBLED", "SetRGBRed"); 
        mcClient->PublishData("*", "RGBLED", "RGBGreen");
        mcClient->SubscribeToCommand("*", "RGBLED", "SetRGBGreen");
        mcClient->PublishData("*", "RGBLED", "RGBBlue");
        mcClient->SubscribeToCommand("*", "RGBLED", "SetRGBBlue");
        
        //END 
        
        Serial.println("Registered");
        return true;  
    } else {
        return false; 
    }
}

//Listen to incoming messages and perform periodic work 
void loop() {
    
    if (!WiFi.ready())
        WiFi.connect();
    
    if (!mcClient->IsConnected()) {
        //If the client is disconnected, reconnect and register
        delay(RECONNECTION_DELAY);
        ConnectAndRegister();       
    } else {
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
        } else if (result == MCClient::BUFFER_PARTIAL) {
            //As soon as a message is arriving, why loop without delay to receive it 
            currentWaitMessageDelay = 0;             
        } else if (result == MCClient::BUFFER_NONE){
            //While nothing is received, before calling again ProcessTCPBuffer(), we progressively wait more and more, until some limit, 
            if (currentWaitMessageDelay < MAXIMUM_WAIT_MESSAGE_DELAY)
                currentWaitMessageDelay += WAIT_MESSAGE_DELAY_INCREMENT; 
        }
        
        //delay(currentWaitMessageDelay);    
    
        //Periodically, we do some work (but if a message is currently received, we loop until we go it all)
        if ((result != MCClient::BUFFER_PARTIAL) && (millis() - lastTime > DO_WORK_PERIOD)) {
            DoWork(); 
            lastTime = millis();
        }  
    }
}

//Process received message
void ProcessReceivedMessage() {
    
    Serial.println("Process message");
    
    if (mcClient->IsConnected()) {
        
        //BEGIN DHT22 DEMO CODE
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
        
        if (message->Name == "RGBLED") {
            
            Serial.println("RGB Command received");
            
            if (message->Parameter == "SetRGBRed") {
                rgbRed = message->Content.toInt() ;
                mcClient->SendData("*", "RGBLED", "RGBRed", String(rgbRed));
            } else if (message->Parameter == "SetRGBGreen") {
                rgbGreen = message->Content.toInt();
                mcClient->SendData("*", "RGBLED", "RGBGreen", String(rgbGreen));
            } else if (message->Parameter == "SetRGBBlue") {
                rgbBlue = message->Content.toInt();
                mcClient->SendData("*", "RGBLED", "RGBBlue", String(rgbBlue));
            }
            RGB.color(rgbRed, rgbGreen, rgbBlue); 
        }
        
        //END
    }
}

void DoWork() {
    Serial.println("Do work");

    //BEGIN DHT22 DEMO CODE
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