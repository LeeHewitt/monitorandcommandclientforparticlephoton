#ifndef __MC_CLIENT_H_
#define __MC_CLIENT_H_

//https://github.com/elcojacobs/spark-firmware/tree/master/inc

#include "spark_wiring.h"
#include "spark_wiring_string.h"
#include "spark_wiring_client.h"
#include "spark_wiring_ipaddress.h"
#include "spark_wiring_tcpclient.h"
#include "message.h"

class MCClient {
    
    private:

        String DeviceName; 
    
        TCPClient* client = NULL;
        
        Message* messageToSend = NULL;
    
        byte *buffer;
        
        size_t bufferOffset; 
        
        void Log(String content);
    
    protected:
    
        void Send(Message* message);
        
        String PadJsonString(String jsonString); 

    public:
    
        static size_t MESSAGE_SIZE;
    
    	static const int BUFFER_READY = 0;
	    static const int BUFFER_NONE = 1;
	    static const int BUFFER_PARTIAL = 2;
	    static const int BUFFER_ERROR = -1;
	    static const int BUFFER_DISCONNECTED = -2;
    
        MCClient(TCPClient* tcpClient); 
        
        bool Connect(IPAddress serverIp, int portNumber);

        bool IsConnected(); 
        
        void Disconnect(); 
        
        void Register(String deviceName);
        void Unregister(String deviceName);
        
        void PublishCommand(String toDevice, String commandTarget, String commandName);
        void PublishData(String toDevice, String dataSource, String dataName);
        void SubscribeToData(String fromDevice, String dataSource, String dataName);
        void SubscribeToCommand(String fromDevice, String commandName, String commandTarget);
        
        void SendCommand(String toDevice, String commandName, String commandTarget, String commandValue);
        void SendData(String toDevice, String dataSource, String dataName, String dataValue);  

        //bool HasMessage();
        
        int BufferMessageData(); 
        
        Message* Read();
};

#endif /* __MC_CLIENT_H_ */