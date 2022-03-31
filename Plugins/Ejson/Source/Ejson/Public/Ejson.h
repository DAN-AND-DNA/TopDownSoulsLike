// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Ejson.generated.h"


class FEjsonModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

UENUM(BlueprintType)
enum class EEjsonType : uint8 {
	JSON_NONE,
	JSON_NULL,
	JSON_STRING,
	JSON_NUMBER,
	JSON_BOOLEAN,
	JSON_ARRAY,
	JSON_OBJECT,
};


UCLASS(BlueprintType)
class EJSON_API UEjsonValue : public UObject {
	GENERATED_BODY()
public:
	
	UEjsonValue();

	static void TestAll();
	
	UFUNCTION(BlueprintCallable)
	void Reset();

	// make

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonString(const FString& Value);

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonNumber(float Value);

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonBoolean(bool Value);

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonNull();

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonArray(const TArray<UEjsonValue*>& Value);

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonObject(const TMap<FString, UEjsonValue*>& Value);

	static UEjsonValue* MakeFromCPPVersion(TSharedPtr<FJsonValue> Value);

	UFUNCTION(BlueprintPure)
		static UEjsonValue* MakeJsonFromString(const FString& Value);



	// get

	UFUNCTION(BlueprintPure)
	bool JsonAsString(FString& Value) const;

	UFUNCTION(BlueprintPure)
	bool JsonAsBoolean(bool& Value) const;

	UFUNCTION(BlueprintPure)
	bool JsonAsNumber(float& Value) const;

	UFUNCTION(BlueprintCallable)
	bool JsonAsArray(TArray<UEjsonValue*>& Value) const;

	UFUNCTION(BlueprintPure)
	bool JsonAsObject(TMap<FString, UEjsonValue*>& Value) const;

	
	UFUNCTION(BlueprintCallable)
	bool SetJsonString(const FString& Value);

	UFUNCTION(BlueprintCallable)
	bool SetJsonBoolean(bool Value);

	UFUNCTION(BlueprintCallable)
	bool SetJsonNumber(float Value);

	UFUNCTION(BlueprintCallable)
	bool SetJsonArray(const TArray<UEjsonValue*>& Value);

	UFUNCTION(BlueprintCallable)
	bool SetJsonObject(const TMap<FString, UEjsonValue*>& Value);

	// for object field
	UFUNCTION(BlueprintCallable)
	bool SetFieldValue(const FString& Field, const UEjsonValue* Value);

	// for object field
	UFUNCTION(BlueprintPure)
	UEjsonValue* GetFieldValue(const FString& Field) const;

	TSharedPtr<FJsonValue> ToCPPVersion() const;

	UFUNCTION(BlueprintPure)
	FString ToString(bool bPretty) const;
public:


	UPROPERTY(BlueprintReadWrite)
	TArray<UEjsonValue*> ValueArray;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, UEjsonValue*> ValueObject;

private:
	 UEjsonValue* MakeFromCPPVersionInternal(TSharedPtr<class FEJsonValue> Value);

private:
	//EEjsonType JsonType;


	TSharedPtr<class FEJsonValue> Raw;

};


EJSON_API TSharedPtr<FJsonValue> HelperParseJSON(const FString& RawJson);
EJSON_API FString HelperStringifyJSON(TSharedPtr<FJsonValue> JsonValue, bool bPretty = false);


