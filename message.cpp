#include "message.h"
#include "stdlib.h"
#include "spark_wiring_usbserial.h"

String Message::SERVER = "Server";
String Message::ALL = "*";
String Message::NOT_AVAILABLE = "[NA]";  

Message::Message(String sendingDevice, String fromDevice, String toDevice, ContentTypes contentType, String name, String parameter, String content) {
    Serial.println("Instanciate message");
    
    SendingDevice = sendingDevice;
    ReceivingDevice = toDevice; //receivingDevice is equal to toDevice unless the server rerouted the message to a device listening for the to/from traffic
    FromDevice = fromDevice;
    ToDevice = toDevice;

    ContentType = contentType;
    Name = name;
    Parameter = parameter;
    Content = content;

    Timestamp = "1900-01-01T00:00:00.0000000-00:00"; //Default value
}

Message::Message(Message &message) :
    Message(message.SendingDevice, message.FromDevice, message.ToDevice, message.ContentType, message.Name, message.Parameter, message.Content) { }

Message::~Message() {
    Serial.println("Destroy message");
    
    delete SendingDevice;
    delete ReceivingDevice;
    delete FromDevice;
    delete ToDevice;
    delete Name;
    delete Parameter;
    delete Content;
    delete Timestamp;
}

Message* Message::InstanciateRegisterMessage(String sendingDevice) {
    return new Message(sendingDevice, sendingDevice, SERVER, ContentTypes::CONTROL, "REGISTER", NOT_AVAILABLE, NOT_AVAILABLE);
}

Message* Message::InstanciateUnregisterMessage(String sendingDevice) {
    return new Message(sendingDevice, NOT_AVAILABLE, SERVER, ContentTypes::CONTROL, "UNREGISTER", NOT_AVAILABLE, NOT_AVAILABLE);
}

Message* Message::InstanciatePublishMessage(String sendingDevice, String toDevice, String publicationSource, String publicationName) {
    return new Message(sendingDevice, sendingDevice, toDevice, ContentTypes::CONTROL, "PUBLISH", publicationSource, publicationName);
}

Message* Message::InstanciateUnpublishMessage(String sendingDevice, String toDevice, String publicationSource, String publicationName) {
    return new Message(sendingDevice, sendingDevice, toDevice, ContentTypes::CONTROL, "UNPUBLISH", publicationSource, publicationName);
}

Message* Message::InstanciateSubscribeMessage(String sendingDevice, String fromDevice, String toDevice, String publicationSource, String publicationName) {
    return new Message(sendingDevice, fromDevice, toDevice, ContentTypes::CONTROL, "SUBSCRIBE", publicationSource, publicationName);
}

Message* Message::InstanciateUnsubscribeMessage(String sendingDevice, String fromDevice, String toDevice, String publicationSource, String publicationName) {
    return new Message(sendingDevice, fromDevice, toDevice, ContentTypes::CONTROL, "UNSUBSCRIBE", publicationSource, publicationName);
}

Message* Message::InstanciateCommandMessage(String sendingDevice, String toDevice, String commandName, String commandTarget, String commandContent) {
    return new Message(sendingDevice, sendingDevice, toDevice, ContentTypes::COMMAND, commandName, commandTarget, commandContent);
}

Message* Message::InstanciateDataMessage(String sendingDevice, String toDevice, String dataSource, String dataName, String dataContent) {
    return new Message(sendingDevice, sendingDevice, toDevice, ContentTypes::DATA, dataSource, dataName, dataContent);
}

Message* Message::InstanciateHeartbeatMessage(String ofDevice) {
    return new Message(ofDevice, ofDevice, SERVER, ContentTypes::HEARTBEAT, "HEARTBEAT", NOT_AVAILABLE, NOT_AVAILABLE);
}

String Message::ToJSONString() {
    String jsonString;
    
    jsonString += "{";  
    jsonString += "\"SendingDevice\":\"" + SendingDevice + "\","; 
    jsonString += "\"ReceivingDevice\":\"" + ReceivingDevice + "\",";
    jsonString += "\"FromDevice\":\"" + FromDevice + "\",";
    jsonString += "\"ToDevice\":\"" + ToDevice + "\",";
    jsonString += "\"ContentType\":" + (String)ContentType + ",";
    jsonString += "\"Name\":\"" + Name + "\",";
    jsonString += "\"Parameter\":\"" + Parameter + "\",";
    jsonString += "\"Content\":\"" + Content + "\",";
    jsonString += "\"Timestamp\":\"" + Timestamp + "\"";
    jsonString += "}"; 

    return jsonString;
}

