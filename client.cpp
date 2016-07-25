#include "client.h"
#include "spark_wiring_usbserial.h"

size_t MCClient::MESSAGE_SIZE = 512;

MCClient::MCClient(TCPClient* tcpClient) {
    this->tcpClient = tcpClient;    
    buffer = (byte*)malloc(MCClient::MESSAGE_SIZE);
    bufferOffset = 0;
}

bool MCClient::Connect(IPAddress serverIp, int portNumber) {
    
    if (tcpClient->connect(serverIp, portNumber)) {
        Serial.println("Connected");
        return true;
    } else {
        return false;
    }
}

bool MCClient::IsConnected() {
    return tcpClient->connected(); 
}

void MCClient::Disconnect() {
    if (tcpClient->connected()) {
        Serial.println("Disconnecting.");
        tcpClient->stop();
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

void MCClient::SubscribeToCommand(String fromDevice, String commandTarget, String commandName) {
    messageToSend = Message::InstanciateSubscribeMessage(DeviceName, fromDevice, DeviceName, commandTarget, commandName);
    Send(messageToSend);
    delete messageToSend;
}

void MCClient::SendCommand(String toDevice, String commandTarget, String commandName, String commandValue) {
    messageToSend = Message::InstanciateCommandMessage(DeviceName, toDevice, commandTarget, commandName, commandValue);
    Send(messageToSend);
    delete messageToSend;
}

void MCClient::SendData(String toDevice, String dataSource, String dataName, String dataValue) {
    messageToSend = Message::InstanciateDataMessage(DeviceName, toDevice, dataSource, dataName, dataValue);
    Send(messageToSend);
    delete messageToSend;
}  

//Taken from https://github.com/rickkas7/fixedlentcprcv/, thanks to Particle community member rickkas7
int MCClient::ProcessTCPBuffer() {
    
	if (tcpClient->connected()) {
		int count = tcpClient->read(&buffer[bufferOffset], MCClient::MESSAGE_SIZE - bufferOffset);
		if (count <= 0) {
			return (bufferOffset == 0) ? BUFFER_NONE : BUFFER_PARTIAL;
		}

		// New data arrived
		bufferOffset += (size_t)count;
		if (bufferOffset < MCClient::MESSAGE_SIZE) {
			//Still not a full message
		    //Serial.printlnf("Partial offset=%d size=%d", bufferOffset, MCClient::MESSAGE_SIZE);
			return BUFFER_PARTIAL;
		}

		// Got a full message
		bufferOffset = 0;
		return BUFFER_READY;
	} 
}

Message* MCClient::Receive() {
    Serial.println("Receive");
    
    //Extract Json content
    String jsonString = GetJsonStringFromBytesBuffer();
    Serial.println(jsonString);
    
    sendingDevice = "";
    receivingDevice = "";
    fromDevice = "";
    toDevice = "";
    name = "";
    parameter = "";
    content = "";
    
    unsigned int valueTokenIndex = 0;
    bool insideValueToken = false;
    unsigned int beginQuoteIndex = 0;
    unsigned int endQuoteIndex = 0;
    
    for (unsigned int index = 0; index < MCClient::MESSAGE_SIZE; index++) {
        char c = jsonString.charAt(index);
        
        if (c == ':') {
            //a value token will start
            valueTokenIndex++;
            insideValueToken = true;
        }
    
        if (insideValueToken) {
            
            //we detect begin/end quote indexes 
            if (c == '"' && beginQuoteIndex == 0) {
                beginQuoteIndex = index;
            }
            else if (c == '"' && endQuoteIndex == 0) {
                endQuoteIndex = index;
            }
            
            if (beginQuoteIndex > 0 && endQuoteIndex > 0) {
                String substring = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                //Serial.println(substring);

                //we extract the value    
                switch(valueTokenIndex) {
                    case sendingDeviceIndex:
                        sendingDevice = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case receivingDeviceIndex:
                        receivingDevice = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case fromDeviceIndex:
                        fromDevice = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case toDeviceIndex:
                        toDevice = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case contentTypeIndex:
                        contentType = (Message::ContentTypes)atoi(jsonString.substring(beginQuoteIndex + 1, endQuoteIndex));
                        break;
                    case nameIndex:
                        name = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case parameterIndex:
                        parameter = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                    case contentIndex:
                        content = jsonString.substring(beginQuoteIndex + 1, endQuoteIndex);
                        break;
                }
                
                insideValueToken = false;
                beginQuoteIndex = 0;
                endQuoteIndex = 0;
            }
        }
    }

    Message* message = new Message(sendingDevice, fromDevice, toDevice, contentType, name, parameter, content);

    return message; 
}

void MCClient::Send(Message *message) {
    Serial.println("Send");
    
    if (IsConnected()) {
        String paddedJsonString = PadJsonString(message->ToJSONString()); 
        tcpClient->write(paddedJsonString);
    }
}

String MCClient::GetJsonStringFromBytesBuffer() {
    //Fill a string object with the content of the bytes array (is there a better way to do that ?)
    String jsonString = "";
    bool appendFlag = false;
    for (int charIndex = 0; charIndex < (int)MCClient::MESSAGE_SIZE; charIndex++) {
        char c = (char)buffer[charIndex];
        
        if (c == '{') {
            appendFlag = true;
        }
        
        if (appendFlag) {
            jsonString = jsonString + (char)buffer[charIndex]; 
        }
        
        if (c == '}') {
            appendFlag = false;
        }
    }

    return jsonString;
}

String MCClient::PadJsonString(String jsonString) {
    for (int i = strlen(jsonString); i < (int)MCClient::MESSAGE_SIZE; i++) {
        jsonString += "."; 
    }
    
    return jsonString;
}




