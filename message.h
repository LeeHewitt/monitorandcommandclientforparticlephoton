#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include "spark_wiring_string.h"
        
class Message
{
    private : 
        
    protected : 

    public :
    
        enum ContentTypes { CONTROL = 0, COMMAND = 1, DATA = 2, HEARTBEAT = 3 };
    
        static String SERVER;
        static String ALL;
        static String NOT_AVAILABLE;   
    
        String SendingDevice;
        String ReceivingDevice;
        String FromDevice;
        String ToDevice;
        ContentTypes ContentType;
        String Name;
        String Parameter;
        String Content;
        String Timestamp;

        Message(String sendingDevice, String fromDevice, String toDevice, ContentTypes contentType, String name, String parameter, String content);
        Message(Message &message);
        ~Message();
        
        static Message* InstanciateRegisterMessage(String sendingDevice);
        static Message* InstanciateUnregisterMessage(String sendingDevice);
        static Message* InstanciatePublishMessage(String sendingDevice, String toDevice, String publicationSource, String publicationName);
        static Message* InstanciateUnpublishMessage(String sendingDevice, String toDevice, String publicationSource, String publicationName);
        static Message* InstanciateSubscribeMessage(String sendingDevice, String fromDevice, String toDevice, String publicationSource, String publicationName);
        static Message* InstanciateUnsubscribeMessage(String sendingDevice, String fromDevice, String toDevice, String publicationSource, String publicationName);
        static Message* InstanciateCommandMessage(String sendingDevice, String toDevice, String commandName, String commandTarget, String commandContent);
        static Message* InstanciateDataMessage(String sendingDevice, String toDevice, String dataSource, String dataName, String dataContent);
        static Message* InstanciateHeartbeatMessage(String ofDevice);

        String ToJSONString();
};

#endif /* __MESSAGE_H_ */
