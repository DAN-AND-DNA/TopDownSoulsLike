#include "ETcpConnection.h"
#include "ETcpPluginSettings.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h"
#include <string>

UETcpConnection::UETcpConnection()
	: SendBufferSize(16384)
	, RecvBufferSize(16384) 
	, TimeBetweenTicks(0.008f)
	, BaseId(1000) {
}

UETcpConnection* UETcpConnection::NewTcpConnMgr() {
	return NewObject<UETcpConnection>();
}


void UETcpConnection::Stop(int32 ConnectedId) {
	auto Worker = this->TcpWorkers.Find(ConnectedId);

	if (Worker == nullptr) {
		return;
	}

	if (Worker->IsValid()) {
		(*Worker)->Stop();
	}

	this->TcpWorkers.Remove(ConnectedId);
}

void UETcpConnection::Reconnect(int32 ConnectedId) {
	auto OldWorker = this->TcpWorkers.Find(ConnectedId);

	if (OldWorker == nullptr) {
		return;
	}

	(*OldWorker)->Reconnect();
}


void UETcpConnection::Connect(
	const FString& IpAddress,
	int32 Port,
	const FOnConnectedDelegate& OnConnected,
	const FOnDisconnectedDelegate& OnDisconnected,
	const FOnMessageBufferDelegate& OnMessage, int32& ConnectedId) {

	this->OnConnectedDelegate = OnConnected;
	this->OnDisconnectedDelegate = OnDisconnected;
	this->OnMessageBufferDelegate = OnMessage;

	//ConnectedId = TcpWorkers.Num();
	this->BaseId++;
	ConnectedId = this->BaseId;
	auto ThisWeakPtr = TWeakObjectPtr<UETcpConnection>(this);
	TSharedPtr<FETcpWorker> NewWorker(new FETcpWorker(IpAddress, Port, ThisWeakPtr, ConnectedId, this->RecvBufferSize, this->SendBufferSize, this->TimeBetweenTicks));
	this->TcpWorkers.Add(ConnectedId, NewWorker);
	NewWorker->Start();
}

void UETcpConnection::K2_Connect(
	const FString& IpAddress,
	int32 Port,
	const FETcpPluginOnConnectedDelegate& OnConnected,
	const FETcpPluginOnDisconnectedDelegate& OnDisconnected,
	const FETcpPluginOnMessageBufferDelegate& OnMessage, int32& ConnectedId) {

	this->Connect(
		IpAddress, 
		Port, 
		FOnConnectedDelegate::CreateLambda([OnConnected](int32 ConnectedId) {OnConnected.ExecuteIfBound(ConnectedId); }),
		FOnDisconnectedDelegate::CreateLambda([OnDisconnected](int32 ConnectedId) {OnDisconnected.ExecuteIfBound(ConnectedId); }),
		FOnMessageBufferDelegate::CreateLambda([OnMessage](int32 ConnectedId) {OnMessage.ExecuteIfBound(ConnectedId); }),
		ConnectedId);
	
}



void UETcpConnection::SendData(int32 ConnectionId, const TArray<uint8>& DataToSend) {
	if (!this->TcpWorkers.Contains(ConnectionId)) {
		return;
	}
	
	if (this->TcpWorkers[ConnectionId]->IsConnected()) {
		this->TcpWorkers[ConnectionId]->Send(DataToSend);		
	}
	 
}


bool UETcpConnection::IsConnected(int32 ConnectionId) const {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		return this->TcpWorkers[ConnectionId]->IsConnected();
	}

	return false;
}


bool UETcpConnection::ReadInt32(int32 ConnectionId, int32& OutInt) {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		return this->TcpWorkers[ConnectionId]->ReadInt32(OutInt);
	}
	return false;
}

bool UETcpConnection::ReadString(int32 ConnectionId, FString& OutString, int32 StringLength) {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		return this->TcpWorkers[ConnectionId]->ReadString(OutString, StringLength);
	}
	return false;
}

bool UETcpConnection::PeekInt32(int32 ConnectionId, int32& OutInt, int32 Offset) {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		return this->TcpWorkers[ConnectionId]->PeekInt32(OutInt, Offset);
	}
	return false;
}

bool UETcpConnection::PeekString(int32 ConnectionId, FString& OutString, int32 BytesLength, int32 Offset) {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		return this->TcpWorkers[ConnectionId]->PeekString(OutString, BytesLength, Offset);
	}
	return false;
}

void UETcpConnection::Consume(int32 ConnectionId, int32 BytesNum) {
	if (this->TcpWorkers.Contains(ConnectionId)) {
		 this->TcpWorkers[ConnectionId]->Consume(BytesNum);
	}
}


TArray<uint8> UETcpConnection::NewBytesBuffer() {
	TArray<uint8> Result;

	return Result;
}

void UETcpConnection::AppendInt32(int32 InInt, TArray<uint8>& BytesBuffer) {
	FETcpWorker::AppendInt32(InInt, BytesBuffer);
}

void UETcpConnection::AppendString(const FString& InString, TArray<uint8>& BytesBuffer) {
	FETcpWorker::AppendString(InString, BytesBuffer);
}

void UETcpConnection::AppendBytes(const TArray<uint8>& SrcBytesBuffer, TArray<uint8>& DestBytesBuffer) {
	FETcpWorker::AppendBytes(SrcBytesBuffer, DestBytesBuffer);
}

TArray<uint8> UETcpConnection::StringToBytes(const FString& InString, int32& OutBytesSize) {
	TArray<uint8> OutBytes;
	OutBytesSize = FETcpWorker::StringToBytes(InString, OutBytes);
	return OutBytes;
}



void UETcpConnection::BeginDestroy() {
	Super::BeginDestroy();

	TArray<int32> ConnectionIds;
	this->TcpWorkers.GetKeys(ConnectionIds);

	for (int32 ConnectionId : ConnectionIds) {
		this->Stop(ConnectionId);
	}
}



void UETcpConnection::PrintToConsole(const FString& InMessage, bool bError) {
	auto Settings = GetDefault<UETcpPluginSettings>();

	if (!Settings) {
		return;
	}

	if (bError && Settings->bPostErrorsToMessageLog) {
		auto NewLog = FMessageLog(TEXT("ETcp Plugin"));
		NewLog.Open(EMessageSeverity::Error, true);
		NewLog.Message(EMessageSeverity::Error, FText::AsCultureInvariant(InMessage));
	} else {
		UE_LOG(LogTemp, Log, TEXT("%s"), *(InMessage));
	}
}

void UETcpConnection::ExecuteOnConnected(int32 WorkId, TWeakObjectPtr<UETcpConnection> ThisObj) {

	// only one game thread !
	if (!ThisObj.IsValid()) {
		return;
	}

	this->OnConnectedDelegate.ExecuteIfBound(WorkId);
}

void UETcpConnection::ExecuteOnDisconnected(int32 WorkerId, TWeakObjectPtr<UETcpConnection> ThisObj) {
	if (ThisObj == nullptr) {
		return;
	}

	// only one game thread !
	if (!ThisObj.IsValid()) {
		return;
	}

	if (!TcpWorkers.Contains(WorkerId)) {
		return;
		//TcpWorkers.Remove(WorkerId);
	}

	this->OnDisconnectedDelegate.ExecuteIfBound(WorkerId);
}


void UETcpConnection::ExecuteOnMessageReceived(int32 WorkerId, TWeakObjectPtr<UETcpConnection> ThisObj) {

	if (!ThisObj.IsValid()) {
		return;
	}

	if (!this->TcpWorkers.Contains(WorkerId)) {
		return;
	}

	///TArray<uint8> Msg;

	//this->TcpWorkers[WorkerId]->ReadFromInputBox(Msg);
	this->OnMessageBufferDelegate.ExecuteIfBound(WorkerId);

}

void FETcpWorker::TestAll() {

	// econde and decode
	{
		{
			// int32
			TArray<uint8> TmpBuffer;
			int32 TestTarget = 37;
			FETcpWorker::AppendInt32(TestTarget, TmpBuffer);
			int32 TempInt = 0;
			FETcpWorker::BytesToInt32_Unsafe(TmpBuffer.GetData(), TempInt);
			ensureAlways(TestTarget == TempInt);
		}

		// 2. string
		{
			TArray<uint8> TmpBuffer;
			FString TestTarget(TEXT(R"({"Name":"dan","Age":7})"));
			FETcpWorker::AppendString(TestTarget, TmpBuffer);
			FTCHARToUTF8 Convert(*TestTarget);
			auto BytesLength = Convert.Length();
			FString TempStr;
			FETcpWorker::BytesToString_Unsafe(TmpBuffer.GetData(), TempStr, BytesLength);
			ensureAlways(TestTarget == TempStr);
		}

		// 3. [int32 string]
		{
			// write
			TArray<uint8> TmpBuffer;
			FString TestTarget(TEXT(R"({"Name":"dan","Age":7})"));
			FTCHARToUTF8 Convert(*TestTarget);
			auto TestTargetLength = Convert.Length();
			FETcpWorker::AppendInt32(TestTargetLength, TmpBuffer);
			FETcpWorker::AppendString(TestTarget, TmpBuffer);

			// read
			int32 TempInt = 0;
			FETcpWorker::BytesToInt32_Unsafe(TmpBuffer.GetData(), TempInt);
			FString TempStr;
			FETcpWorker::BytesToString_Unsafe(TmpBuffer.GetData() + 4, TempStr, TempInt);
			ensureAlways(TestTarget == TempStr);
		}
	}

	// api
	{
		FString FakeIpAddress = "127.0.0.1";
		int32 FakePort = 3000;
		FString ErrorMsg;
		

		// connect
		TSharedRef<FETcpWorker> TestWorker(new FETcpWorker(FakeIpAddress, FakePort, nullptr, 10001, 2000, 2000, 0.001));
		ensureAlways(TestWorker->Init());
		ensureAlways(TestWorker->Connect(ErrorMsg, true));
		ensureAlways(TestWorker->IsConnected());


		FString	SendedJsonMsg = TEXT(R"({"Name":"dan","Age":7,"level":999})");
		TArray<uint8> FakeRecvData;
		TArray<uint8> BytesJsonMsg;
		auto SendedJsonMsgSize = FETcpWorker::StringToBytes(SendedJsonMsg, BytesJsonMsg);
		FETcpWorker::AppendInt32(SendedJsonMsgSize, FakeRecvData);
		FETcpWorker::AppendBytes(BytesJsonMsg, FakeRecvData);

		// recv
		int32 BytesNum = 0 ;
		ensureAlways(TestWorker->TryRecv(BytesNum, ErrorMsg, true, FakeRecvData.GetData(), FakeRecvData.Num()));
		ensureAlways(TestWorker->GetInputBufferSize() == FakeRecvData.Num());

		// handle msg
		int32 RecvedJsonMsgSize = 0;
		FString RecvedJsonMsg;
		ensureAlways(TestWorker->PeekInt32(RecvedJsonMsgSize));
		ensureAlways(RecvedJsonMsgSize == SendedJsonMsgSize);
		ensureAlways(TestWorker->PeekString(RecvedJsonMsg, RecvedJsonMsgSize, 4));
		ensureAlways(RecvedJsonMsg == SendedJsonMsg);
		TestWorker->Consume(RecvedJsonMsgSize + 4);
		
		// check input buffer
		ensureAlways(TestWorker->GetInputBufferSize() == 0);
		TestWorker->ResetBuffer();
		ensureAlways(TestWorker->GetInputBufferSize() == 0);


		// send
		ensureAlways(TestWorker->GetOutputBufferSize() == 0);
		TestWorker->Send(FakeRecvData);
		ensureAlways(TestWorker->GetOutputBufferSize() == FakeRecvData.Num());
		ensureAlways(TestWorker->TrySend(ErrorMsg, true));
		ensureAlways(TestWorker->GetOutputBufferSize() == 0);
		TestWorker->ResetBuffer();
		ensureAlways(TestWorker->GetOutputBufferSize() == 0);

		TestWorker->Exit();
	}
}



FETcpWorker::FETcpWorker(
	const FString& InIp,
	int32 InPort,
	TWeakObjectPtr<UETcpConnection> InOwner,
	int32 InId,
	int32 InRecvBufferSize,
	int32 InSendBufferSize,
	float InTimeBetweenTicks)

	: IpAddress(InIp)
	, Port(InPort)
	, ThreadSpawnActor(InOwner)
	, Id(InId)
	, RecvBufferSize(InRecvBufferSize)
	, SendBufferSize(InSendBufferSize)
	, bRun(false)
	, bConnected(false)
	, Thread(nullptr) 
	, TimeBetweenTicks(InTimeBetweenTicks)
	, InputReadIndex(0)
	, InputWriteIndex(0)
	, OutputReadIndex(0)
	, OutputWriteIndex(0)
	, InputBufferCriticalSection()
	, OutputBufferCriticalSection()
	, DoReconnectCriticalSection()
	, bFirst(false) {
}



FETcpWorker::~FETcpWorker() {
	this->Stop();
	if (!this->Thread) {
		return;
	}

	this->Thread->WaitForCompletion();
	delete this->Thread;
	this->Thread = nullptr;
}


void FETcpWorker::Start() {
	check(!Thread && "Thread wasn't null at the start!");
	this->Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("FETcpWorker %s:%d"), *(this->IpAddress), this->Port), 128 * 1024, TPri_Normal);
	UE_LOG(LogTemp, Log, TEXT("Create new Thread"));
}


void FETcpWorker::Send(const TArray<uint8>& Message) {
	auto BytesNum = Message.Num();
	if (BytesNum == 0 ) {
		return;
	}

	FScopeLock OutputBufferLock(&(this->OutputBufferCriticalSection));	// lock
	this->OutputBuffer.SetNumUninitialized(this->OutputWriteIndex + BytesNum, false);
	FMemory::Memcpy(this->OutputBuffer.GetData() + this->OutputWriteIndex, Message.GetData(), BytesNum);
	OutputBufferLock.Unlock();

	this->OutputWriteIndex += BytesNum;
}

void FETcpWorker::AppendInt32(int32 InInt, TArray<uint8>& Dest) {
	if (InInt < 0) {
		InInt = 0;
	}

	auto InUint = (uint32)(InInt);

	auto StartPos = Dest.Num();
	Dest.SetNumUninitialized(StartPos + 4);
	FMemory::Memcpy(Dest.GetData() + StartPos, (uint8*)(&InUint), 4);
	
}

void FETcpWorker::AppendString(const FString& InString, TArray<uint8>& Dest) {
	FTCHARToUTF8 Convert(*InString);
	auto BytesLength = Convert.Length();

	auto StartPos = Dest.Num();
	Dest.SetNumUninitialized(StartPos + BytesLength);

	// just is TCHAR_TO_UTF8(str)
	auto Src = (uint8*)(Convert.Get());
	FMemory::Memcpy(Dest.GetData() + StartPos, Src, BytesLength);
}
void FETcpWorker::AppendBytes(const TArray<uint8>& SrcBytesBuffer, TArray<uint8>& DestBytesBuffer) {
	auto BytesLength = SrcBytesBuffer.Num();
	auto StartPos = DestBytesBuffer.Num();
	DestBytesBuffer.SetNumUninitialized(StartPos + BytesLength);
	FMemory::Memcpy(DestBytesBuffer.GetData() + StartPos, SrcBytesBuffer.GetData(), BytesLength);
}

void  FETcpWorker::BytesToInt32_Unsafe(uint8* Src, int32& Result) {
	Result = *(int32*)(Src);
}

void FETcpWorker::BytesToString_Unsafe(uint8* Bytes, FString& OutString, int32 BytesLength) {
	TArray<uint8> Dest;
	Dest.SetNumUninitialized(BytesLength + 1);

	FMemory::Memcpy(Dest.GetData(), Bytes, BytesLength);
	auto c = '\0';
	FMemory::Memcpy(Dest.GetData() + BytesLength, &c, 1);
	OutString = FString(UTF8_TO_TCHAR(Dest.GetData()));
}


int32 FETcpWorker::StringToBytes(const FString& InString, TArray<uint8>& OutBytesBuffer) {
	FTCHARToUTF8 Convert(*InString);
	auto BytesLength = Convert.Length();
	OutBytesBuffer.SetNumUninitialized(BytesLength, true);

	// just is TCHAR_TO_UTF8(str)
	auto Src = (uint8*)(Convert.Get());
	FMemory::Memcpy(OutBytesBuffer.GetData(), Src, BytesLength);

	return BytesLength;
}


bool FETcpWorker::PeekInt32(int32& OutInt, int32 Offset){
	FScopeLock InputBufferLock(&(this->InputBufferCriticalSection)); // lock

	if (this->InputWriteIndex - (this->InputReadIndex + Offset) < 4) {
		OutInt = 0;
		return false;
	}

	InputBufferLock.Unlock();

	FETcpWorker::BytesToInt32_Unsafe(this->InputBuffer.GetData() + this->InputReadIndex + Offset, OutInt);

	return true;
}

bool FETcpWorker::PeekString(FString& OutString, int32 BytesLength, int32 Offset) {
	FScopeLock InputBufferLock(&(this->InputBufferCriticalSection));

	if (this->InputWriteIndex - (this->InputReadIndex + Offset) < BytesLength) {
		return false;
	}

	FETcpWorker::BytesToString_Unsafe(this->InputBuffer.GetData() + this->InputReadIndex + Offset, OutString, BytesLength);

	return true;
}


void FETcpWorker::Consume(int32 BytesNum) {
	FScopeLock InputBufferLock(&(this->InputBufferCriticalSection));
	this->InputReadIndex += BytesNum;
}

bool FETcpWorker::ReadInt32(int32& OutInt) {
	if (!this->PeekInt32(OutInt)) {
		return false;
	}

	this->Consume(4);
	return true;
}

bool FETcpWorker::ReadString(FString& OutString, int32 BytesLength) {
	if (!this->PeekString(OutString, BytesLength)) {
		return false;
	}

	this->Consume(BytesLength);
	return true;
}

bool FETcpWorker::Init() {
	this->bRun = true;
	this->bConnected = false;
	return true;
}

uint32 FETcpWorker::Run() {
	AsyncTask(ENamedThreads::GameThread, []() {
		UETcpConnection::PrintToConsole(TEXT("Starting Tcp socket thread"), false);
	});

	this->bDoReconnect = true; // first run
	this->bConnected = false;

	while (this->bRun) {
		FDateTime timeBeginningOfTick = FDateTime::UtcNow();

		// 0. do reconnect
		if (this->bDoReconnect) {

			this->SocketShutdown();
			this->bConnected = false;
		
			FString ErrorMsg;
			if (!this->Connect(ErrorMsg)) {
				AsyncTask(ENamedThreads::GameThread, [ErrorMsg]() {
					UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
				});
				this->bConnected = false;

				// notify disconnect;
				AsyncTask(ENamedThreads::GameThread, [this]() {
					if (!this->ThreadSpawnActor.IsValid()) {
						return;
					}

					this->ThreadSpawnActor->ExecuteOnDisconnected(this->Id, this->ThreadSpawnActor);
				});


			} else {
				this->bConnected = true;
				AsyncTask(ENamedThreads::GameThread, [this]() {
					if (!this->ThreadSpawnActor.IsValid()) {
						return;
					}

					// notify connected
					this->ThreadSpawnActor->ExecuteOnConnected(this->Id, this->ThreadSpawnActor);
				});
			}

			// clear cache
			this->InputReadIndex = 0;
			this->InputWriteIndex = 0;
			this->OutputReadIndex = 0;
			this->OutputWriteIndex = 0;

			this->bDoReconnect = false;
		}

		// 1. check connected;
		if (!this->bConnected) {
			FDateTime timeEndOfTick = FDateTime::UtcNow();
			FTimespan tickDuration = timeEndOfTick - timeBeginningOfTick;
			float secondsThisTickTook = tickDuration.GetTotalSeconds();
			float timeToSleep = TimeBetweenTicks - secondsThisTickTook;
			if (timeToSleep > 0.f) {
				FPlatformProcess::Sleep(timeToSleep);
			}

			continue;
		}

		// 2. reset buffer
		this->ResetBuffer();


		// 3. recv
		int32 OnceReadedBytes = 0;
		FString ErrorMsg;
		if (!this->TryRecv(OnceReadedBytes, ErrorMsg)) {
			AsyncTask(ENamedThreads::GameThread, [ErrorMsg]() {
				UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
			});

			// notify disconnect;
			AsyncTask(ENamedThreads::GameThread, [this]() {
				if (!this->ThreadSpawnActor.IsValid()) {
					return;
				}

				this->ThreadSpawnActor->ExecuteOnDisconnected(this->Id, this->ThreadSpawnActor);
			});

			this->bConnected = false;
			continue;
		}

		if (!this->bFirst &&OnceReadedBytes > 0) {
			this->bFirst = false;
		}
		
		
		// 4. notify msg
		if (OnceReadedBytes > 0) {
			AsyncTask(ENamedThreads::GameThread, [this]() {
				if (!this->ThreadSpawnActor.IsValid()) {
					return;
				}

				this->ThreadSpawnActor->ExecuteOnMessageReceived(this->Id, this->ThreadSpawnActor);
			});
		}
		
		// 5. send
		if (!this->TrySend(ErrorMsg)) {
			AsyncTask(ENamedThreads::GameThread, [ErrorMsg]() {
				UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
			});

			// notify disconnect;
			AsyncTask(ENamedThreads::GameThread, [this]() {
				if (!this->ThreadSpawnActor.IsValid()) {
					return;
				}

				this->ThreadSpawnActor->ExecuteOnDisconnected(this->Id, this->ThreadSpawnActor);
			});

			this->bConnected = false;
			continue;
		} 
		
		// 6. wait
		FDateTime timeEndOfTick = FDateTime::UtcNow();
		FTimespan tickDuration = timeEndOfTick - timeBeginningOfTick;
		float secondsThisTickTook = tickDuration.GetTotalSeconds();
		float timeToSleep = TimeBetweenTicks - secondsThisTickTook;
		if (timeToSleep > 0.f)
		{
			FPlatformProcess::Sleep(timeToSleep);
		}
		
		// 7. check disconnect
		if (!this->bFirst) {
			int32 TmpBytesRead;
			uint8 TmpDummy;
			if (!this->Socket->Recv(&TmpDummy, 1, TmpBytesRead, ESocketReceiveFlags::Peek)) {

				// notify disconnect;
				AsyncTask(ENamedThreads::GameThread, [this]() {
					if (!this->ThreadSpawnActor.IsValid()) {
						return;
					}

					this->ThreadSpawnActor->ExecuteOnDisconnected(this->Id, this->ThreadSpawnActor);
				});

				this->bConnected = false;
				continue;
			}
		}
	}

	return 0;
}

void FETcpWorker::Stop() {
	this->bRun = false;
}

void FETcpWorker::Exit() {
	this->bConnected = false;
	this->bRun = false;
	this->SocketShutdown();

	AsyncTask(ENamedThreads::GameThread, []() {
		UE_LOG(LogTemp, Log, TEXT("Tcp socket thread was destroyed"));
	});
}

bool FETcpWorker::IsConnected() const {
	return this->bConnected;
}

bool FETcpWorker::Connect(FString& ErrorMsg, bool bFakeTest) {

	this->Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	if (!this->Socket) {
		ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d" "CreateSocket failed"), __LINE__);
		return false;
	}

	check(this->Socket->SetNoDelay(true));
	check(this->Socket->SetReceiveBufferSize(this->RecvBufferSize, this->ActualRecvBufferSize));
	check(this->Socket->SetSendBufferSize(this->SendBufferSize, this->ActualSendBufferSize));

	FIPv4Address Ip;
	if (!FIPv4Address::Parse(this->IpAddress, Ip)) {
		FString BadIpAddress = this->IpAddress;	// copy one for game thread
		ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d" "Bad IpAddress"), __LINE__);
		return false;
	}

	TSharedRef<FInternetAddr> InternetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(this->Port);

	if (bFakeTest) {
		check(this->Socket->SetNonBlocking(false));
		check(this->Socket->SetNonBlocking(true));
		this->bConnected = true;
		return true;
	}

	check(this->Socket->SetNonBlocking(false));
	if (!this->Socket->Connect((*InternetAddr))) {
		check(this->Socket->SetNonBlocking(true));
		ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d " "Connected to server failed"), __LINE__);
		return false;
	}
	check(this->Socket->SetNonBlocking(true));
	this->bConnected = true;
	return true;
}


void FETcpWorker::Reconnect() {
	this->bDoReconnect = true;
}

void FETcpWorker::SocketShutdown() {
	if(!this->Socket) {
		return;
	}

	this->Socket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(this->Socket);
	this->Socket = nullptr;
}

bool FETcpWorker::TryRecv(int32& BytesNum, FString& ErrorMsg, bool bFakeTest, uint8* FakeRecvData, uint32 FakeRecvDataSize) {
	if (!this->Socket) {
		ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d" "Raw socket is nullptr"), __LINE__);
		return false;
	}

	uint32 PendingDataSize = 0;
	BytesNum = 0;

	while (this->bRun) {
		if (bFakeTest) {
			PendingDataSize = FakeRecvDataSize;
		} else {
			if (!this->Socket->HasPendingData(PendingDataSize)) {
				break;
			}
		}

		FScopeLock InputBufferLock(&(this->InputBufferCriticalSection));	// lock
		this->InputBuffer.SetNumUninitialized(this->InputWriteIndex + PendingDataSize, false);

		int32 BytesReaded = 0;
		if (bFakeTest) {
			FMemory::Memcpy(this->InputBuffer.GetData() + this->InputWriteIndex, FakeRecvData, PendingDataSize);
			BytesReaded = PendingDataSize;
		} else {
			if (!this->Socket->Recv(this->InputBuffer.GetData() + this->InputWriteIndex, PendingDataSize, BytesReaded)) {
				ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d" "Raw socket recv failed"), __LINE__);
				return false;
			}
		}

		if (BytesReaded <= 0 ) {
			break;
		}

		this->InputWriteIndex += BytesReaded;
		BytesNum = BytesReaded;

		if (bFakeTest) {
			break;
		}
	}

	return true;
}

bool FETcpWorker::TrySend(FString& ErrorMsg, bool bFakeTest) {
	if (!this->Socket) {
		return false;
	}

	int32 BytesSent = 0;
	while ((this->OutputWriteIndex - this->OutputReadIndex) > 0) {

		FScopeLock OutputBufferLock(&(this->OutputBufferCriticalSection));

		if (bFakeTest) {
			BytesSent = this->OutputWriteIndex - this->OutputReadIndex;
		}
		else {
			if (!this->Socket->Send(this->OutputBuffer.GetData() + this->OutputReadIndex, this->OutputWriteIndex - this->OutputReadIndex, BytesSent)) {
				ErrorMsg = FString::Printf(TEXT(__FILE__ ":%d" "CreateSocket failed"), __LINE__);
				return false;
			}
		}

		if (BytesSent <= 0) {
			break;
		}

		this->OutputReadIndex += BytesSent;
	}

	//ensureAlways(TestWorker->GetInputBufferSize() == 0);


	// send
	//ensureAlways(TestWorker->GetOutputBufferSize() == 0);

	

	return true;
}

void FETcpWorker::ResetBuffer() {
	//UE_LOG(LogTemp, Log, TEXT("INPUT: %d"), GetInputBufferSize());
	//UE_LOG(LogTemp, Log, TEXT("0UTPUT: %d"), GetOutputBufferSize());

	if (this->InputWriteIndex != 0) {
		FScopeLock InputBufferLock(&(this->InputBufferCriticalSection));
		if (this->InputReadIndex == this->InputWriteIndex) {
			this->InputReadIndex = 0;
			this->InputWriteIndex = 0;

			if (this->InputBuffer.Num() > 4000) {
				this->InputBuffer.SetNumUninitialized(4000, true);
				UE_LOG(LogTemp, Log, TEXT("SET INPUT 4000"));
			}
		}

		
	}

	if (this->OutputWriteIndex != 0) {
		FScopeLock OutputBufferLock(&(this->OutputBufferCriticalSection));
		if (this->OutputWriteIndex == this->OutputReadIndex) {
			this->OutputWriteIndex = 0;
			this->OutputReadIndex = 0;	

			if (this->OutputBuffer.Num() > 4000) {
				this->OutputBuffer.SetNumUninitialized(4000, true);
				UE_LOG(LogTemp, Log, TEXT("SET OUTPUT 4000"));
			}
		}
	}
}

int32 FETcpWorker::GetInputBufferSize()  {
	FScopeLock InputBufferLock(&(this->InputBufferCriticalSection));
	return this->InputWriteIndex - this->InputReadIndex;
}

int32 FETcpWorker::GetOutputBufferSize() {
	FScopeLock OutputBufferLock(&(this->OutputBufferCriticalSection));
	return this->OutputWriteIndex - this->OutputReadIndex;
}