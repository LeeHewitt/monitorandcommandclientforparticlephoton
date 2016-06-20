#ifndef __MC_CLIENT_H_
#define __MC_CLIENT_H_

#include "spark_wiring.h"
#include "spark_wiring_string.h"
#include "spark_wiring_client.h"
#include "spark_wiring_ipaddress.h"
#include "spark_wiring_tcpclient.h"
#include "message.h"

class MCClient {
    
    private:

        String DeviceName; 
        TCPClient* tcpClient = NULL;
        Message* messageToSend = NULL;

        byte *buffer;
        size_t bufferOffset; 
    
        static const int sendingDeviceIndex = 1;
        static const int receivingDeviceIndex = 2;
        static const int fromDeviceIndex = 3;
        static const int toDeviceIndex = 4;
        static const int contentTypeIndex = 5;
        static const int nameIndex = 6;
        static const int parameterIndex = 7;
        static const int contentIndex = 8;
    
        String sendingDevice = "";
        String receivingDevice = "";
        String fromDevice = "";
        String toDevice = "";
        Message::ContentTypes contentType;
        String name = "";
        String parameter = "";
        String content = "";
    
        String GetJsonStringFromBytesBuffer();
        String PadJsonString(String jsonString); 
    
    protected:

    public:

        static size_t MESSAGE_SIZE;
    
    	static const int BUFFER_READY = 0;
	    static const int BUFFER_NONE = 1;
	    static const int BUFFER_PARTIAL = 2;
	    static const int BUFFER_ERROR = -1;
	    static const int BUFFER_DISCONNECTED = -2;
    
        MCClient(TCPClient* tcpClient); 
        
        bool Connect(IPAddress serverIp, int portNumber);
        void Disconnect(); 
        bool IsConnected(); 
        
        void Register(String deviceName);
        void Unregister(String deviceName);
        
        void PublishCommand(String toDevice, String commandTarget, String commandName);
        void PublishData(String toDevice, String dataSource, String dataName);
        void SubscribeToData(String fromDevice, String dataSource, String dataName);
        void SubscribeToCommand(String fromDevice, String commandName, String commandTarget);
        
        void SendCommand(String toDevice, String commandName, String commandTarget, String commandValue);
        void SendData(String toDevice, String dataSource, String dataName, String dataValue);  
        
        int ProcessTCPBuffer(); 
        
        Message* Receive();
        void Send(Message* message);
};

#endif /* __MC_CLIENT_H_ */