#include "client.h"
#include "spark_wiring_usbserial.h"

size_t MCClient::MESSAGE_SIZE = 512;

MCClient::MCClient(TCPClient* tcpClient) {
    this->client = tcpClient;    
    buffer = (byte*)malloc(MCClient::MESSAGE_SIZE);
    bufferOffset = 0;
}

bool MCClient::Connect(IPAddress serverIp, int portNumber) {
    
    if (client->connect(serverIp, portNumber))
    {
        Serial.println("Connected");
        return true;
    }
    else
    {
        return false;
    }
}

bool MCClient::IsConnected() {
    return client->connected(); 
}

void MCClient::Disconnect() {
    if (client->connected())
    {
        Serial.println("Disconnecting.");
        client->stop();
    }
}

void MCClient::Register(String deviceName) {
    DeviceName = deviceName; 
    messageToSend = Message::InstanciateRegisterMessage(DeviceName);
    Send(messageToSend);        
    delete messageToSend;
}
 
void MCClient::Unregister(String deviceName) {
    messageToSend = Message::InstanciateUnregisterMessage(deviceName);
    Send(messageToSend); 
    delete messageToSend;
}

void MCClient::PublishCommand(String toDevice, String commandTarget, String commandName) {
    messageToSend = Message::InstanciatePublishMessage(DeviceName, toDevice, commandTarget, commandName);
    Send(messageToSend); 
    delete messageToSend;
}

void MCClient::PublishData(String toDevice, String dataSource, String dataName) {
    messageToSend = Message::InstanciatePublishMessage(DeviceName, toDevice, dataSource, dataName);
    Send(messageToSend); 
    delete messageToSend;
}

void MCClient::SubscribeToData(String fromDevice, String dataSource, String dataName) {
    messageToSend = Message::InstanciateSubscribeMessage(DeviceName, fromDevice, DeviceName, dataSource, dataName);
    Send(messageToSend);        
    delete messageToSend;
}

void MCClient::SubscribeToCommand(String fromDevice, String commandName, String commandTarget) {
    messageToSend = Message::InstanciateSubscribeMessage(DeviceName, fromDevice, DeviceName, commandName, commandTarget);
    Send(messageToSend);
    delete messageToSend;
}

void MCClient::SendCommand(String toDevice, String commandName, String commandTarget, String commandValue) {
    messageToSend = Message::InstanciateCommandMessage(DeviceName, toDevice, commandName, commandTarget, commandValue);
    Send(messageToSend);
    delete messageToSend;
}

void MCClient::SendData(String toDevice, String dataSource, String dataName, String dataValue) {
    messageToSend = Message::InstanciateDataMessage(DeviceName, toDevice, dataSource, dataName, dataValue);
    Send(messageToSend);
    delete messageToSend;
}  

void MCClient::Send(Message *message) {
    if (IsConnected()) {
        String jsonString = message->ToJSONString();
        String paddedJsonString = PadJsonString(jsonString); 
        client->write(paddedJsonString);
    }
}

bool MCClient::HasMessage() {
    return (client->available() > (int)MCClient::MESSAGE_SIZE); //DOES NOT WORK
}

int MCClient::IsAvailable() {
    return client->available(); //DOES NOT WORK
}

//Taken from https://github.com/rickkas7/fixedlentcprcv/, thanks to rickkas7
int MCClient::BufferMessageData() {
    
	if (client == NULL) {
		return BUFFER_DISCONNECTED;
	}

	if (client->connected()) {
		int count = client->read(&buffer[bufferOffset], MCClient::MESSAGE_SIZE - bufferOffset);
		if (count <= 0) {
			return (bufferOffset == 0) ? BUFFER_NONE : BUFFER_PARTIAL;
		}

		// New data arrived
		bufferOffset += (size_t)count;
		if (bufferOffset < MCClient::MESSAGE_SIZE) {
			// Still not a full message
			Serial.printlnf("Partial offset=%d size=%d", bufferOffset, MCClient::MESSAGE_SIZE);
			return BUFFER_PARTIAL;
		}

		// Got a full message
		bufferOffset = 0;
		return BUFFER_READY;
	}
	else {
		client = NULL;
		return BUFFER_DISCONNECTED;
	}
}

Message* MCClient::Read() {
    Serial.println("Reading message");
    
    Message* message = NULL; 

    /*
    if (HasMessage()) {

        Log ("Data available");

        //unsigned char* messageChars = new unsigned char[MCClient::MESSAGE_SIZE]; 

        bool appendFlag = false;
        int charIndex = 0;
        
        while(client->available()) {
            char c = client->read();
            Serial.print(c);    
            
            if (c == '.') {
                continue;
            } else if (c == '{') {
                messageChars[charIndex++] = c;
                appendFlag = true;
            } else if (c == '}') {
                messageChars[charIndex++] = c;
                appendFlag = false;   
            } else {
                if (appendFlag) {
                    messageChars[charIndex++] = c;
                }
            }
        }
    */
    
    //Fill a string object with the content of the bytes array
    String paddedJsonString = "";
    for (int charIndex = 0; charIndex < (int)MCClient::MESSAGE_SIZE; charIndex++)
    {
        paddedJsonString = paddedJsonString + (char)buffer[charIndex];     
    }
    Serial.println(paddedJsonString);
    
    //delete messageChars; 
    //messageChars = NULL; 

    int lastClosingBraceIndex = paddedJsonString.indexOf('}'); 
    String jsonString = paddedJsonString.substring(0, lastClosingBraceIndex); 
    Serial.println(jsonString); 
    
    message = new Message(jsonString);
    //}
    
    return message; 
}


String MCClient::PadJsonString(String jsonString) {

    for (int i = strlen(jsonString); i < (int)MCClient::MESSAGE_SIZE; i++) {
        jsonString += "."; 
    }
    
    return jsonString;
}

//https://github.com/drcheap/spark-tcptest/blob/master/tcptest.ino




