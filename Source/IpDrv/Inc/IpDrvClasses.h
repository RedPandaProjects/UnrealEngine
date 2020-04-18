/*===========================================================================
	C++ class definitions exported from UnrealScript.

   This is automatically generated using 'Unreal.exe -make -h'
   DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/
#pragma pack (push,4)
#ifdef IPDRV_EXPORTS
#define IPDRV_API DLL_EXPORT
#else
#define IPDRV_API DLL_IMPORT
#endif


#ifndef NAMES_ONLY
#define DECLARE_NAME(name) extern IPDRV_API FName IPDRV_##name;
#endif

DECLARE_NAME(Accepted)
DECLARE_NAME(Connected)
DECLARE_NAME(Closed)
DECLARE_NAME(ReceivedBinary)
DECLARE_NAME(ReceivedText)
DECLARE_NAME(ReceivedLine)
DECLARE_NAME(Resolved)
DECLARE_NAME(ResolveFailed)

#ifndef NAMES_ONLY

enum ETcpMode
{
    TMOD_Binary             =0,
    TMOD_Text               =1,
    TMOD_Line               =2,
    TMOD_MAX                =3,
};

enum ETcpLinkState
{
    TCP_Closed              =0,
    TCP_Resolving           =1,
    TCP_Connecting          =2,
    TCP_Listening           =3,
    TCP_Open                =4,
    TCP_MAX                 =5,
};

class IPDRV_API ATcpLink : public AInfo
{
public:
    CHAR URL[80];
    BYTE IP[4];
    INT Port;
    FLOAT KeepaliveSeconds;
    FLOAT KeepaliveCounter;
    FLOAT TimeoutSeconds;
    FLOAT TimeoutCounter;
    BYTE TcpInternal[64];
    INT MainSocket;
    INT ConnectSocket;
    DWORD bSocketInitialized:1;
    DWORD bSocketBound:1;
    DWORD bWSAInitialized:1;
    BYTE LinkState;
    BYTE LinkMode;
    void execGetIPByName( FFrame& Stack, BYTE*& Result );
    void execEncrypt( FFrame& Stack, BYTE*& Result );
    void execGetLastError( FFrame& Stack, BYTE*& Result );
    void execReadBinary( FFrame& Stack, BYTE*& Result );
    void execSendBinary( FFrame& Stack, BYTE*& Result );
    void execReadText( FFrame& Stack, BYTE*& Result );
    void execSendText( FFrame& Stack, BYTE*& Result );
    void execClose( FFrame& Stack, BYTE*& Result );
    void execOpen( FFrame& Stack, BYTE*& Result );
    void execListen( FFrame& Stack, BYTE*& Result );
    void eventReceivedLine(const CHAR* S)
    {
        struct {CHAR S[240]; } Parms;
        appStrncpy(Parms.S,S,240);
        ProcessEvent(FindFunctionChecked(IPDRV_ReceivedLine),&Parms);
    }
    void eventReceivedText(const CHAR* S)
    {
        struct {CHAR S[240]; } Parms;
        appStrncpy(Parms.S,S,240);
        ProcessEvent(FindFunctionChecked(IPDRV_ReceivedText),&Parms);
    }
    void eventReceivedBinary(INT Count)
    {
        struct {INT Count; } Parms;
        Parms.Count=Count;
        ProcessEvent(FindFunctionChecked(IPDRV_ReceivedBinary),&Parms);
    }
    void eventClosed()
    {
        ProcessEvent(FindFunctionChecked(IPDRV_Closed),NULL);
    }
    void eventConnected()
    {
        ProcessEvent(FindFunctionChecked(IPDRV_Connected),NULL);
    }
    void eventAccepted()
    {
        ProcessEvent(FindFunctionChecked(IPDRV_Accepted),NULL);
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ATcpLink,AInfo,0|CLASS_Transient)
    #include "ATcpLink.h"
};

enum EUdpMode
{
    UDP_Text                =0,
    UDP_Binary              =1,
    UDP_MAX                 =2,
};

#define UCONST_BroadcastAddr -1

class IPDRV_API AUdpLink : public AInfo
{
public:
    INT Socket;
    BYTE UdpMode;
    void execIpAddrToURL( FFrame& Stack, BYTE*& Result );
    void execSendBinary( FFrame& Stack, BYTE*& Result );
    void execSendText( FFrame& Stack, BYTE*& Result );
    void execBindPort( FFrame& Stack, BYTE*& Result );
    void execResolve( FFrame& Stack, BYTE*& Result );
    void eventReceivedText(FIpAddr Addr, const CHAR* Text)
    {
        struct {FIpAddr Addr; CHAR Text[240]; } Parms;
        Parms.Addr=Addr;
        appStrncpy(Parms.Text,Text,240);
        ProcessEvent(FindFunctionChecked(IPDRV_ReceivedText),&Parms);
    }
    void eventResolveFailed()
    {
        ProcessEvent(FindFunctionChecked(IPDRV_ResolveFailed),NULL);
    }
    void eventResolved(FIpAddr Addr)
    {
        struct {FIpAddr Addr; } Parms;
        Parms.Addr=Addr;
        ProcessEvent(FindFunctionChecked(IPDRV_Resolved),&Parms);
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AUdpLink,AInfo,0|CLASS_Transient)
    #include "AUdpLink.h"
};

#undef DECLARE_NAME
#endif
#pragma pack (pop)
