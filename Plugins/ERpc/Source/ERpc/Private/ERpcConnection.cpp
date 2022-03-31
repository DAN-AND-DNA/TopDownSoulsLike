#include "ERpcConnection.h"
#include "ETcpConnection.h"
#include "Json.h"


FERpcError FERpcError::Ok{ true, FString(), 0 };
FERpcError FERpcError::RequestTimeout{ false, FString(TEXT("request timeout")), 0 };
FERpcError FERpcError::Disconnected{ false, FString(TEXT("current is disconnected")), 0 };


UERpcConnection::UERpcConnection()
	: TcpConnMgr(nullptr)
	, BaseRequestId(0)
	, RunningTime(0)
	, LastReconnectTime(0)
	, CurrentTcpConnectId(0)
	, RetryConnectDelay(3)
	, bAllowedToTick(false)
	, bConnected(false)
	, bFirstConnected(true)
	, bIsReconnecting(false) {
}

void UERpcConnection::BeginDestroy() {
	Super::BeginDestroy();

	this->bAllowedToTick = false;
	this->Disconnect();
}


FERpcError UERpcConnection::Connect(const FString& InIp, int32 InPort) {
	if (InIp.IsEmpty()) {
		return FERpcError{ false, FString(TEXT("Server ip is empty!")) };
	}

	if (InPort == 0) {
		return FERpcError{ false, FString(TEXT("Server port is 0!")) };
	}


	if (this->Ip == InIp && this->Port == InPort) {
		return FERpcError::Ok;
	}

	this->Ip = InIp;
	this->Port = InPort;

	if (this->TcpConnMgr == nullptr) {
		this->TcpConnMgr = UETcpConnection::NewTcpConnMgr();
	}
	this->TcpConnMgr->Stop(this->CurrentTcpConnectId);

	int32 ConnectId = 0;

	this->TcpConnMgr->Connect(
		this->Ip,
		this->Port,
		this->TcpConnMgr->OnConnected().CreateUObject(this, &UERpcConnection::TcpOnConnectedInternal),
		this->TcpConnMgr->OnDisconnected().CreateUObject(this, &UERpcConnection::TcpOnDisconnectedInternal),
		this->TcpConnMgr->OnMessage().CreateUObject(this, &UERpcConnection::TcpOnMessageInternal),
		ConnectId
	);

	check(ConnectId > 0);
	this->CurrentTcpConnectId = ConnectId;
	this->bAllowedToTick = true;

	return FERpcError::Ok;
}

void UERpcConnection::Disconnect() {
	if (this->TcpConnMgr == nullptr) {
		return;
	}

	this->TcpConnMgr->OnDisconnected().Unbind();
	this->TcpConnMgr->OnConnected().Unbind();
	this->TcpConnMgr->OnMessage().Unbind();

	this->TcpConnMgr->Stop(this->CurrentTcpConnectId);
}

FERpcError UERpcConnection::K2_Request(const FString& Method, const FString& Params, FRpcOnResponseDelegate OnResponse, FString& OutRequestId, int32 MaxWaitSeconds) {
	return this->Request(
		Method,
		Params,
		FOnResponseDelegate::CreateLambda([this, OnResponse](TSharedPtr<FJsonValue> Response, FERpcError RpcError) {

			FString StrResponse;
			auto JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StrResponse);
			
			if (Response.IsValid()) {
				if (!FJsonSerializer::Serialize(Response, FString(), JsonWriter)) {
					if (StrResponse.Len() > 0 && StrResponse[0] == TEXT(',')) {
						StrResponse.RemoveAt(0, 1, false);
					}
				}
			}

			OnResponse.ExecuteIfBound(this, StrResponse, RpcError); 
		}),
		OutRequestId, 
		MaxWaitSeconds
	);
}

FERpcError UERpcConnection::Request(const FString& Method, const FString& Params, FOnResponseDelegate OnResponse, FString& OutRequestId, int32 MaxWaitSeconds) {
	if (!this->bConnected) {
		return FERpcError::Disconnected;
	}

	if (Method.IsEmpty()) {
		return FERpcError{ false, FString(TEXT("Method is empty")) };
	}

	if (Params.IsEmpty()) {
		return FERpcError {false, FString(TEXT("Params is empty"))};
	}

	auto RequestId = this->GetNextRequestId();

	// register
	UERpcConnection::FRequest Request;
	Request.Method = Method;
	Request.OnResponse = OnResponse;
	Request.ExpireTime = MaxWaitSeconds + this->RunningTime;
	this->Requests.Add(RequestId, MoveTemp(Request));

	// send to server
	FString JsonMsg = FString::Printf(TEXT(R"({"id":"%s","method":"%s","params":%s})"), *RequestId, *Method, *Params);
	int32 JsonMsgSize = 0;
	auto BytesJsonMsg = UETcpConnection::StringToBytes(JsonMsg, JsonMsgSize);
	check(JsonMsgSize > 0);

	TArray<uint8> TotalMsg;
	
	UETcpConnection::AppendInt32(JsonMsgSize, TotalMsg);
	UETcpConnection::AppendBytes(BytesJsonMsg, TotalMsg);
	this->TcpConnMgr->SendData(this->CurrentTcpConnectId, TotalMsg);

	OutRequestId = RequestId;
	return FERpcError::Ok;
}

FERpcError UERpcConnection::k2_Notify(const FString& Method, const FString& Params) {
	return this->Notify(Method, Params);
}

FERpcError UERpcConnection::Notify(const FString& Method, const FString& Params) {
	if (!this->bConnected) {
		return FERpcError::Disconnected;
	}

	if (Method.IsEmpty()) {
		return FERpcError{ false, FString(TEXT("Method is empty")) };
	}

	if (Params.IsEmpty()) {
		return FERpcError{ false, FString(TEXT("Params is empty")) };
	}

	FString JsonMsg = FString::Printf(TEXT(R"({"method":"%s","params":%s})"), *Method, *Params);
	int32 JsonMsgSize = 0;
	auto BytesJsonMsg = UETcpConnection::StringToBytes(JsonMsg, JsonMsgSize);
	check(JsonMsgSize > 0);

	TArray<uint8> TotalMsg;

	UETcpConnection::AppendInt32(JsonMsgSize, TotalMsg);
	UETcpConnection::AppendBytes(BytesJsonMsg, TotalMsg);
	this->TcpConnMgr->SendData(this->CurrentTcpConnectId, TotalMsg);

	return FERpcError::Ok;
}


FERpcError UERpcConnection::k2_RegisterNotificationCallback(const FString& Method, FRpcOnNotifiedDelegate Callback) {
	return this ->RegisterNotificationCallback(
		Method, 
		FOnNotifiedDelegate::CreateLambda([this, Callback](TSharedPtr<FJsonValue> Params) {
			FString StrParams;
			auto JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StrParams);
			if (!FJsonSerializer::Serialize(Params, FString(), JsonWriter)) {
				if (StrParams.Len() > 0 && StrParams[0] == TEXT(',')) {
					StrParams.RemoveAt(0, 1, false);
				}
			}
			Callback.ExecuteIfBound(this, StrParams);
		}));
}

FERpcError UERpcConnection::RegisterNotificationCallback(const FString& Method, FOnNotifiedDelegate Callback) {
	if (Method.IsEmpty()) {
		return FERpcError{ false, FString(TEXT("Method is empty")) };
	}
	
	FNotify Notify;
	Notify.Method = Method;
	Notify.Callback = Callback;

	this->Notifies.Add(Method, Notify);

	return  FERpcError::Ok;
}


void UERpcConnection::Tick(float DeltaTime) {
	if (!this->bAllowedToTick) {
		return;
	}

	this->RunningTime += DeltaTime;

	// 1. reconnect
	if (!this->bIsReconnecting && !this->bConnected) {
		if (this->RunningTime - this->LastReconnectTime >= RetryConnectDelay) {
			this->LastReconnectTime = this->RunningTime;
			UE_LOG(LogTemp, Log, TEXT("do reconnect"));
			this->bIsReconnecting = true;
			this->TcpConnMgr->Reconnect(this->CurrentTcpConnectId);
		}
	}

	// TODO
	// 2. check request timeout
}

TStatId UERpcConnection::GetStatId() const {
	return TStatId();
}


bool UERpcConnection::IsAllowedToTick() const {
	return this->bAllowedToTick;
}

bool UERpcConnection::IsTickableWhenPaused() const {
	return true;
}


void UERpcConnection::TcpOnConnectedInternal(int32 ConnectedId) {
	UE_LOG(LogTemp, Log, TEXT("connected"));
	this->bIsReconnecting = false;
	this->bConnected = true;
	
	// clean msg
	this->Requests.Empty();
	this->K2_OnConnected(bFirstConnected);
	this->bFirstConnected = false;
}

void UERpcConnection::TcpOnDisconnectedInternal(int32 ConnectedId) {
	UE_LOG(LogTemp, Log, TEXT("disconnected"));
	this->bIsReconnecting = false;
	this->bConnected = false;

	this->K2_OnDisconnected();
}

void UERpcConnection::TcpOnMessageInternal(int32 ConnectedId) {
	// 1. parser msg to json
	static FString FieldError(TEXT("error"));
	static FString FieldErrorCode(TEXT("code"));
	static FString FieldErrorMessage(TEXT("message"));

	static FString FieldMethod(TEXT("method"));
	static FString FieldId(TEXT("id"));
	static FString FieldResult(TEXT("result"));
	static FString FieldParams(TEXT("params"));
	
	int32 JsonMsgSize = 0;
	if (!this->TcpConnMgr->PeekInt32(this->CurrentTcpConnectId, JsonMsgSize)) {
		return;
	}
	//UE_LOG(LogTemp, Log, TEXT("%d"), JsonMsgSize);
	FString RawJsonMsg;
	if (!this->TcpConnMgr->PeekString(this->CurrentTcpConnectId, RawJsonMsg, JsonMsgSize, 4)) {
		return;
	}

	this->TcpConnMgr->Consume(this->CurrentTcpConnectId, 4 + JsonMsgSize);
	//UE_LOG(LogTemp, Log, TEXT("%s"), *RawJsonMsg);

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonMsg);
	TSharedPtr<FJsonObject> JsonMsgObject;
	if (!FJsonSerializer::Deserialize(JsonReader, JsonMsgObject) || !JsonMsgObject.IsValid()) {
		 return;
	}

	FString Id;
	FString Method;
	// 2. find msg's callback
	if (JsonMsgObject->TryGetStringField(FieldMethod, Method)) {
		// TODO
		// 3. notify from server
		auto Notify = this->Notifies.Find(Method);
		if (Notify == nullptr) {
			return;
		}
		Notify->Callback.ExecuteIfBound(JsonMsgObject->TryGetField(FieldParams));	
	} else if (JsonMsgObject->TryGetStringField(FieldId, Id)) {
		// 3. exec request callback when get result
		auto Request =  this->Requests.Find(Id);
		if (Request == nullptr) {
			return;
		}

		FERpcError RpcError;
		RpcError.bOk = true;

		if (JsonMsgObject->HasField(FieldError)) {
			auto JsonError = JsonMsgObject->GetObjectField(FieldError);
			RpcError.bOk = false;
			RpcError.Code = JsonError->GetIntegerField(FieldErrorCode);
			RpcError.Message = JsonError->GetStringField(FieldErrorMessage);

			(*Request).OnResponse.ExecuteIfBound(nullptr, RpcError);
		} else {
			auto JsonResult = JsonMsgObject->TryGetField(FieldResult);

			(*Request).OnResponse.ExecuteIfBound(JsonResult, RpcError);
		}


		this->Requests.Remove(Id);
	}
}


FString UERpcConnection::GetNextRequestId() {
	this->BaseRequestId++;
	this->BaseRequestId %= 0xFFFFFFF;
	
	return FString::FromInt(this->BaseRequestId);
}