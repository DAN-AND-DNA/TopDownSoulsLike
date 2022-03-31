#pragma once

#include "CoreMinimal.h"
#include "ETcpConnection.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FETcpPluginOnConnectedDelegate, int32, ConnectedId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FETcpPluginOnDisconnectedDelegate, int32, ConnectedId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FETcpPluginOnMessageBufferDelegate, int32, ConnectedId);


// run at game thread
UCLASS()
class ETCPPLUGIN_API UETcpConnection : public UObject {
	GENERATED_BODY()

public:
	UETcpConnection();

	UFUNCTION(BlueprintPure, Category = "ETcp Plugin")
	static UETcpConnection* NewTcpConnMgr();

	UFUNCTION(BlueprintCallable, Category = "ETcp Plugin")
	void Stop(int32 ConnectedId);

	UFUNCTION(BlueprintCallable, Category = "ETcp Plugin")
	void Reconnect(int32 ConnectedId);

	DECLARE_DELEGATE_OneParam(FOnConnectedDelegate, int32);
	DECLARE_DELEGATE_OneParam(FOnDisconnectedDelegate, int32);
	DECLARE_DELEGATE_OneParam(FOnMessageBufferDelegate, int32);

	void Connect(
		const FString& IpAddress,
		int32 Port,
		const FOnConnectedDelegate& OnConnected,
		const FOnDisconnectedDelegate& OnDisconnected,
		const FOnMessageBufferDelegate& OnMessage, int32& ConnectedId);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect"), Category = "ETcp Plugin")
	void K2_Connect(
		const FString& IpAddress, 
		int32 Port, 
		const FETcpPluginOnConnectedDelegate& OnConnected, 
		const FETcpPluginOnDisconnectedDelegate& OnDisconnected, 
		const FETcpPluginOnMessageBufferDelegate& OnMessage, int32& ConnectedId);

	UFUNCTION(BlueprintCallable, Category = "ETcp Plugin")
	void SendData(int32 ConnectionId, const TArray<uint8>& DataToSend);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ETcp Plugin")
	bool IsConnected(int32 ConnectionId) const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "New Bytes Buffer"), Category = "ETcp Plugin")
	static TArray<uint8> NewBytesBuffer();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Append Int"), Category = "ETcp Plugin")
	static void AppendInt32(int32 Result, UPARAM(ref) TArray<uint8>& DestBytesBuffer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Append String"), Category = "ETcp Plugin")
	static void AppendString(const FString& Result, UPARAM(ref) TArray<uint8>& DestBytesBuffer);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Append Bytes"), Category = "ETcp Plugin")
	static void AppendBytes(const TArray<uint8>& SrcBytesBuffer, UPARAM(ref) TArray<uint8>& DestBytesBuffer);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "String To Bytes"), Category = "ETcp Plugin")
	static TArray<uint8> StringToBytes(const FString& InString, int32& OutBytesSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Int"), Category = "ETcp Plugin")
	bool ReadInt32(int32 ConnectionId, int32& OutInt);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read String"), Category = "ETcp Plugin")
	bool ReadString(int32 ConnectionId, FString& OutString, int32 StringLength);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Peek Int"), Category = "ETcp Plugin")
	bool PeekInt32(int32 ConnectionId, int32& OutInt, int32 Offset = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Peek String"), Category = "ETcp Plugin")
	bool PeekString(int32 ConnectionId, FString& OutString, int32 BytesLength, int32 Offset = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Consume"), Category = "ETcp Plugin")
	void Consume(int32 ConnectionId, int32 BytesNum);

	static void PrintToConsole(const FString& Message, bool bError);
	void ExecuteOnConnected(int32 WorkId, TWeakObjectPtr<UETcpConnection> ThisObj);
	void ExecuteOnDisconnected(int32 WorkId, TWeakObjectPtr<UETcpConnection> ThisObj);
	void ExecuteOnMessageReceived(int32 WorkdId, TWeakObjectPtr<UETcpConnection> ThisObj);

	FOnConnectedDelegate& OnConnected() { return this->OnConnectedDelegate; };
	FOnDisconnectedDelegate& OnDisconnected() { return this->OnDisconnectedDelegate; };
	FOnMessageBufferDelegate& OnMessage() { return this->OnMessageBufferDelegate; }
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ETcp Plugin")
	int32 SendBufferSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ETcp Plugin")
	int32 RecvBufferSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ETcp Plugin")
	float TimeBetweenTicks;

protected:
	virtual void BeginDestroy() override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	TMap<int32, TSharedPtr<class FETcpWorker>> TcpWorkers;

	//FETcpPluginOnConnectedDelegate OnConnectedDelegate;
	//FETcpPluginOnDisconnectedDelegate OnDisconnectedDelegate;
	//FETcpPluginOnMessageBufferDelegate OnMessageBufferDelegate;

	FOnConnectedDelegate OnConnectedDelegate;
	FOnDisconnectedDelegate OnDisconnectedDelegate;
	FOnMessageBufferDelegate OnMessageBufferDelegate;
	int32 BaseId;
};

// run at our worker threads
class FETcpWorker : public FRunnable, public TSharedFromThis<FETcpWorker> {

public:
	FETcpWorker(const FString& InIp,
				int32 InPort,
				TWeakObjectPtr<UETcpConnection> InOwner, 
				int32 InId, 
				int32 InRecvBufferSize, 
				int32 InSendBufferSize, 
				float InTimeBetweenTicks);
	
	virtual ~FETcpWorker();
	void static TestAll();
	void Start();
	void Send(const TArray<uint8>& Message);

	static void AppendInt32(int32 InInt, TArray<uint8>& BytesBuffer);
	static void AppendString(const FString& InString, TArray<uint8>& DestBytesBuffer);
	static void AppendBytes(const TArray<uint8>& SrcBytesBuffer, TArray<uint8>& DestBytesBuffer);
	static int32 StringToBytes(const FString& InString, TArray<uint8>& OutBytesBuffer);

	static void BytesToInt32_Unsafe(uint8* BytesBuffer, int32&OutInt);
	static void BytesToString_Unsafe(uint8* BytesBuffer, FString& OutString, int32 BytesLength);

	bool PeekInt32(int32& OutInt, int32 Offset = 0);
	bool PeekString( FString& OutString, int32 BytesLength, int32 Offset = 0);
	void Consume(int32 BytesNum);

	bool ReadInt32(int32& Result);
	bool ReadString(FString& Result, int32 BytesLength);

	virtual bool Init() override;	// init
	virtual uint32 Run() override; // loop
	virtual void Stop() override;  // mananul stop
	virtual void Exit() override; // clean

	void SocketShutdown();

	bool IsConnected() const;
	bool Connect(FString& ErrorMsg, bool bFakeTest = false);
	void Reconnect();


private:
	bool TryRecv(int32& BytesNum, FString& ErrorMsg, bool bFakeTest = false, uint8* FakeRecvData = nullptr, uint32 FakeRecvDataSize = 0);
	bool TrySend(FString& ErrorMsg, bool bFakeTest = false);
	void ResetBuffer();
	int32 GetInputBufferSize();
	int32 GetOutputBufferSize();

private:
	FString IpAddress;
	int Port;
	TWeakObjectPtr<UETcpConnection> ThreadSpawnActor;
	int32 Id;
	int32 RecvBufferSize;
	int32 SendBufferSize;
	FThreadSafeBool bRun;
	FThreadSafeBool bConnected;
	FRunnableThread* Thread;
	class FSocket* Socket = nullptr;
	int32 ActualRecvBufferSize;
	int32 ActualSendBufferSize;
	float TimeBetweenTicks;

	TArray<uint8> InputBuffer;
	int32 InputReadIndex;
	int32 InputWriteIndex;

	TArray<uint8> OutputBuffer;
	int32 OutputReadIndex;
	int32 OutputWriteIndex;

	FCriticalSection InputBufferCriticalSection;
	FCriticalSection OutputBufferCriticalSection;
	FCriticalSection DoReconnectCriticalSection;

	bool bFirst;
	bool bDoReconnect;
};
