#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "ERpcConnection.generated.h"


USTRUCT(BlueprintType)
struct ERPC_API FERpcError {
	GENERATED_BODY()
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOk = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Message;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Code = 0;

	static FERpcError Ok;
	static FERpcError RequestTimeout;
	static FERpcError Disconnected;
};


DECLARE_DYNAMIC_DELEGATE_ThreeParams(FRpcOnResponseDelegate, UERpcConnection*, Connection, FString, Response, FERpcError, RpcError);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRpcOnNotifiedDelegate, UERpcConnection*, Connection, FString, Params);

// run at game thread
UCLASS(Blueprintable, BlueprintType)
class  ERPC_API UERpcConnection : public UObject, public FTickableGameObject {
	GENERATED_BODY()

public:
	UERpcConnection();
	virtual void BeginDestroy() override;

	DECLARE_DELEGATE_TwoParams(FOnResponseDelegate, TSharedPtr<FJsonValue> /*response*/, FERpcError /*error*/);
	DECLARE_DELEGATE_OneParam(FOnNotifiedDelegate, TSharedPtr<FJsonValue> /*params*/);

	UFUNCTION(BlueprintCallable, Category = "ERpc")
	FERpcError Connect(const FString& Ip, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "ERpc")
	void Disconnect();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Connected"), Category = "ERpc")
	void K2_OnConnected(bool bReconnected);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Disconnected"), Category = "ERpc")
	void K2_OnDisconnected();
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request"), Category = "ERpc")
	FERpcError K2_Request(const FString& Method, const FString& Params, FRpcOnResponseDelegate OnResponse, FString& OutRequestId, int32 MaxWaitSeconds = 10);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Notify"), Category = "ERpc")
	FERpcError k2_Notify(const FString& Method, const FString& Params);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Register Notification Callback"), Category = "ERpc")
	FERpcError k2_RegisterNotificationCallback(const FString& Method, FRpcOnNotifiedDelegate Callback);

	FERpcError Request(const FString& Method, const FString& Params, FOnResponseDelegate OnResponse, FString& OutRequestId, int32 MaxWaitSeconds = 10);

	FERpcError Notify(const FString& Method, const FString& Params);

	FERpcError RegisterNotificationCallback(const FString& Method, FOnNotifiedDelegate Callback);

	void CancelRequest(int32 RequestId);

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsAllowedToTick() const;
	virtual bool IsTickableWhenPaused() const override;

protected:
	virtual void TcpOnConnectedInternal(int32 ConnectedId);
	virtual void TcpOnDisconnectedInternal(int32 ConnectedId);
	virtual void TcpOnMessageInternal(int32 ConnectedId);

private:
	FString GetNextRequestId();

protected:
	struct FRequest {
		FString Method;
		FOnResponseDelegate OnResponse;
		int32 ExpireTime;
	};

	struct FNotify {
		FString Method;
		FOnNotifiedDelegate Callback;
	};

private:
	FString Ip;
	int32 Port;

	UPROPERTY()
	class UETcpConnection* TcpConnMgr;

	int32 BaseRequestId;
	TMap<FString, FRequest> Requests;
	TMap<FString, FNotify> Notifies;

	float RunningTime;
	float LastReconnectTime;
	int32 CurrentTcpConnectId;
	float RetryConnectDelay;
	bool bAllowedToTick;
	bool bConnected;
	bool bFirstConnected;
	bool bIsReconnecting;
};



