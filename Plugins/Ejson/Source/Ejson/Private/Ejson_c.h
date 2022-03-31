// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Core/Public/Containers/UnrealString.h"



class FJsonValue;


enum class EEJsonType : uint8 {
	JSON_NONE,
	JSON_NULL,
	JSON_STRING,
	JSON_NUMBER,
	JSON_BOOLEAN,
	JSON_ARRAY,
	JSON_OBJECT,
};



class  FEJsonValue  {
	
public:
	FEJsonValue();

	static void TestAll();

	static void BecnhAll();
	
	void Reset();

	bool GetString(FString& Value) const;

	
	bool GetBoolean(bool& Value) const;

	
	bool GetNumber(float& Value) const;

	
	bool GetArray(TArray<TSharedPtr<FEJsonValue>>& Value) const;


	bool GetObject(TMap<FString, TSharedPtr<FEJsonValue>>& Value) const;


	
 	bool SetString(const FString& Value);

	
	bool SetBoolean(bool Value);

	
	bool SetNumber(float Value);

	
	bool SetNull();

	
	bool SetArray(const TArray<TSharedPtr<FEJsonValue>>& Value);

	
	bool SetObject(const TMap<FString, TSharedPtr<FEJsonValue>>& Value);

	
	
	static TSharedPtr<FEJsonValue> MakeString(const FString& Value);

	
	static TSharedPtr<FEJsonValue> MakeNumber(float Value);

	
	static TSharedPtr<FEJsonValue> MakeBoolean(bool Value);

	
	static TSharedPtr<FEJsonValue> MakeNull();

	
	static TSharedPtr<FEJsonValue> MakeArray(const TArray<TSharedPtr<FEJsonValue>>& Value);

	
	static TSharedPtr<FEJsonValue> MakeObject(const TMap<FString, TSharedPtr<FEJsonValue>>& Value);

	static TSharedPtr<FEJsonValue> MakeFromCPPVersion(TSharedPtr<FJsonValue> Value);

	
	static TSharedPtr<FEJsonValue> MakeFromString(const FString& Value);


	
	bool SetFieldValue(const FString& Field, const TSharedPtr<FEJsonValue> Value);
	
	
	bool SetFieldString(const FString& Field, const FString& Value);

	
	bool SetFieldNumber(const FString& Field, float Value);

	
	bool SetFieldBoolean(const FString& Field, bool Value);

	
	bool SetFieldNull(const FString& Field);

	
	bool SetFieldArray(const FString& Field, const TArray<TSharedPtr<FEJsonValue>>& Value);

	
	bool SetFieldObject(const FString& Field, const TMap<FString, TSharedPtr<FEJsonValue>>& Value);


	TSharedPtr<FEJsonValue> GetFieldValue(const FString& Field) const;

	
	bool GetFieldString(const FString& Field, FString& Value) const;

	
	bool GetFieldNumber(const FString& Field, float& Value) const;

	
	bool GetFieldBoolean(const FString& Field, bool& Value) const;

	
	bool GetFieldArray(const FString& Field, TArray<TSharedPtr<FEJsonValue>>& Value) const;

	
	bool GetFieldObject(const FString& Field, TMap<FString, TSharedPtr<FEJsonValue>>& Value) const;

	TSharedPtr<FJsonValue> ToCPPVersion() const;

	
	FString ToString(bool bPretty) const;

	EEJsonType GetJsonType() const;

	void SetJsonType(EEJsonType Type);
public:
	FString ValueString;

	
	bool ValueBool;

	
	float ValueNumber;

	
	TArray<TSharedPtr<FEJsonValue>> ValueArray;

	
	TMap<FString, TSharedPtr<FEJsonValue>> ValueObject;

private:
	EEJsonType JsonType;
};


TSharedPtr<FJsonValue> HelperParseJSON_c(const FString& RawJson);
FString HelperStringifyJSON_c(TSharedPtr<FJsonValue> JsonValue, bool bPretty = false);
