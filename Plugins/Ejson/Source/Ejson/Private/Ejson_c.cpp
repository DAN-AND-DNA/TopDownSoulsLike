// Copyright Epic Games, Inc. All Rights Reserved.
#include "Ejson_c.h"
#include <thread>
#include <chrono>

#include "Json.h"



FEJsonValue::FEJsonValue(){
	Reset();
}


void FEJsonValue::BecnhAll() {

	for (int i = 0; i < 1000000; i++) {

		//auto DD = new(FEJsonValue);
		//DD->SetString(FString("ddddddddddddddddddddddddd"));
	
		
	auto EJsonVal = FEJsonValue::MakeFromString(FString("{\"name\":\"danyang\"}"));
	ensureAlways(EJsonVal != nullptr);

	FString Result;
	ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_OBJECT);
	ensureAlways(EJsonVal->GetFieldString(FString("name"), Result) == true);
	ensureAlways(Result == FString("danyang"));
	
	}	
}

void FEJsonValue::TestAll() {

#pragma region deserialize test
	
	{
		auto EJsonVal = FEJsonValue::MakeFromString(FString("null"));
		ensureAlways(EJsonVal != nullptr);
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_NULL);
	}

	{
		auto EJsonVal = FEJsonValue::MakeFromString(FString("{}"));
		ensureAlways(EJsonVal != nullptr);
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_OBJECT);
		ensureAlways(1 == 1);
	}
	

	{
		//for (int i = 0; i < 100000; i++) {
		auto EJsonVal = FEJsonValue::MakeFromString(FString("{\"name\":\"danyang\"}"));
		ensureAlways(EJsonVal != nullptr);

		FString Result;
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_OBJECT);
		ensureAlways(EJsonVal->GetFieldString(FString("name"), Result) == true);
		ensureAlways(Result == FString("danyang"));
		//}
	}

	{	
		auto EJsonVal = FEJsonValue::MakeFromString(FString(R"({"UserInfo":{"Name":"danyang","Age":"37"},"Message":"ok","Code":0})"));
		ensureAlways(EJsonVal != nullptr);
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_OBJECT);

		TMap<FString, TSharedPtr<FEJsonValue>> ResultUserInfo;
		ensureAlways(EJsonVal->GetFieldObject(FString("UserInfo"), ResultUserInfo) == true);

		// get
		FString ResultName;
		ensureAlways(ResultUserInfo["Name"]->GetString(ResultName) == true);
		ensureAlways(ResultName == FString("danyang"));

		//set
		ResultUserInfo["Name"] = FEJsonValue::MakeString(FString("lulu"));
		ensureAlways(EJsonVal->SetFieldObject(FString("UserInfo"), ResultUserInfo) == true);
		ensureAlways(EJsonVal->GetFieldObject(FString("UserInfo"), ResultUserInfo) == true);
		ensureAlways(ResultUserInfo["Name"]->GetString(ResultName) == true);
		ensureAlways(ResultName == FString("lulu"));
	}
	
	{
		auto EJsonVal = FEJsonValue::MakeFromString(FString("[]"));
		ensureAlways(EJsonVal != nullptr);
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_ARRAY);
	}

	{
		auto EJsonVal = FEJsonValue::MakeFromString(FString("[1,2,3,4,5,6,7]"));
		ensureAlways(EJsonVal != nullptr);
		ensureAlways(EJsonVal->JsonType == EEJsonType::JSON_ARRAY);
		TArray<TSharedPtr<FEJsonValue>> Result;
		ensureAlways(EJsonVal->GetArray(Result) == true);
		float FResult;
		ensureAlways(Result[6]->GetNumber(FResult) == true);
		ensureAlways(FResult == 7);
	}
	
#pragma endregion

#pragma region serialize test
	{
		auto EjsonVal = FEJsonValue::MakeString(FString(""));
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_STRING);
		ensureAlways(EjsonVal->ToString(false).Equals(FString(R"("")")));
	}

	{
		auto EjsonVal = FEJsonValue::MakeNull();
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_NULL);
		ensureAlways(EjsonVal->ToString(false) == FString("null"));
	}

	{
		
		auto EjsonVal = FEJsonValue::MakeArray(TArray<TSharedPtr<FEJsonValue>>());
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_ARRAY);
		ensureAlways(EjsonVal->ToString(false).Equals(FString("[]")));
		
	}

	{
		
		
		auto TmpArray = TArray<TSharedPtr<FEJsonValue>>();
		TmpArray.Emplace(FEJsonValue::MakeNumber(1));
		TmpArray.Emplace(FEJsonValue::MakeNumber(2));
		TmpArray.Emplace(FEJsonValue::MakeNumber(3));
		auto EjsonVal = FEJsonValue::MakeArray(TmpArray);
		
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_ARRAY);
		ensureAlways(EjsonVal->ToString(false) == (FString("[1,2,3]")));

	}
	
	{
		auto EjsonVal = FEJsonValue::MakeObject(TMap<FString, TSharedPtr<FEJsonValue>>());
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_OBJECT);
		ensureAlways(EjsonVal->ToString(false).Equals(FString("{}")));
	}

	{
		auto TmpObj = TMap<FString, TSharedPtr<FEJsonValue>>();
		TmpObj.Emplace(FString("name"), FEJsonValue::MakeString(FString("dan")));
		auto EjsonVal = FEJsonValue::MakeObject(TmpObj);
		ensureAlways(EjsonVal->JsonType == EEJsonType::JSON_OBJECT);
		ensureAlways(EjsonVal->ToString(false).Equals(FString(R"({"name":"dan"})")));
	}

#pragma endregion

	UE_LOG(LogTemp, Log, TEXT("Ejson module: all tests done!"));
}


void FEJsonValue::Reset() {
	this->JsonType = EEJsonType::JSON_NONE;
	this->ValueString.Reset();
	this->ValueBool = false;
	this->ValueNumber = 0.f;
	this->ValueObject.Reset();
	this->ValueArray.Reset();
}

bool FEJsonValue::GetString(FString& Value) const {
	if (this->JsonType != EEJsonType::JSON_STRING) {
		return false;
	}

	Value = this->ValueString;
	return true;
}

bool FEJsonValue::GetBoolean(bool& Value) const {
	if (this->JsonType != EEJsonType::JSON_BOOLEAN) {
		return false;
	}

	Value = this->ValueBool;
	return true;
}

bool FEJsonValue::GetNumber(float& Value) const {
	if (this->JsonType != EEJsonType::JSON_NUMBER) {
		return false;
	}

	Value = this->ValueNumber;
	return true;
}

bool FEJsonValue::GetArray(TArray<TSharedPtr<FEJsonValue>>& Value) const {
	if (this->JsonType != EEJsonType::JSON_ARRAY) {
		return false;
	}

	Value = this->ValueArray;
	return true;
}

bool FEJsonValue::GetObject(TMap<FString, TSharedPtr<FEJsonValue>>& Value) const {
	if (this->JsonType != EEJsonType::JSON_OBJECT) {
		return false;
	}

	Value = this->ValueObject;
	return true;
}


bool FEJsonValue::SetString(const FString& Value) {
	if (this->JsonType != EEJsonType::JSON_STRING) {
		return false;
	}

	this->ValueString = Value;
	return true;
}

bool FEJsonValue::SetBoolean(bool Value) {
	if (this->JsonType != EEJsonType::JSON_BOOLEAN) {
		return false;
	}

	this->ValueBool = Value;
	return true;
}

bool FEJsonValue::SetNumber(float Value) {
	if (this->JsonType != EEJsonType::JSON_NUMBER) {
		return false;
	}

	this->ValueNumber = Value;
	return true;
}

bool FEJsonValue::SetNull() {
	if (this->JsonType != EEJsonType::JSON_NULL) {
		return false;
	}
	this->Reset();
	this->JsonType = EEJsonType::JSON_NULL;
	return true;
}


bool FEJsonValue::SetArray(const TArray<TSharedPtr<FEJsonValue>>& Value) {
	if (this->JsonType != EEJsonType::JSON_ARRAY) {
		return false;
	}

	this->Reset();
	this->JsonType = EEJsonType::JSON_ARRAY;
	this->ValueArray = Value;
	return true;
}

bool FEJsonValue::SetObject(const TMap<FString, TSharedPtr<FEJsonValue>>& Value) {
	if (this->JsonType != EEJsonType::JSON_OBJECT) {
		return false;
	}

	this->Reset();
	this->JsonType = EEJsonType::JSON_OBJECT;
	this->ValueObject = Value;
	return true;
}

TSharedPtr<FEJsonValue>  FEJsonValue::MakeString(const FString& Value) {
	auto NewJsonValue = MakeShared<FEJsonValue>();
	
	NewJsonValue->JsonType = EEJsonType::JSON_STRING;
	NewJsonValue->ValueString = Value;

	return NewJsonValue;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeNumber(float Value) {
	auto NewJsonValue = MakeShared<FEJsonValue>();

	NewJsonValue->JsonType = EEJsonType::JSON_NUMBER;
	NewJsonValue->ValueNumber = Value;

	return NewJsonValue;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeBoolean( bool Value) {
	auto NewJsonValue = MakeShared<FEJsonValue>();

	NewJsonValue->JsonType = EEJsonType::JSON_BOOLEAN;
	NewJsonValue->ValueBool = Value;

	return NewJsonValue;
}


TSharedPtr<FEJsonValue> FEJsonValue::MakeNull() {
	auto NewJsonValue = MakeShared<FEJsonValue>();

	NewJsonValue->JsonType = EEJsonType::JSON_NULL;

	return NewJsonValue;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeArray(const TArray<TSharedPtr<FEJsonValue>>& Value) {
	auto NewJsonValue = MakeShared<FEJsonValue>();

	NewJsonValue->JsonType = EEJsonType::JSON_ARRAY;
	NewJsonValue->ValueArray = Value;

	return NewJsonValue;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeObject(const TMap<FString, TSharedPtr<FEJsonValue>>& Value) {
	auto NewJsonValue = MakeShared<FEJsonValue>();

	NewJsonValue->JsonType = EEJsonType::JSON_OBJECT;
	NewJsonValue->ValueObject = Value;

	return NewJsonValue;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeFromCPPVersion(TSharedPtr<FJsonValue> Value) {
	if (!Value.IsValid()) {
		return nullptr;
	}

	switch (Value->Type) {
	case EJson::None:
	{
		return nullptr;
	}
	case EJson::Null:
	{
		return FEJsonValue::MakeNull();
	}
	case EJson::String:
	{
		return FEJsonValue::MakeString(Value->AsString());
	}
	case EJson::Number:
	{
		return FEJsonValue::MakeNumber(static_cast<float>(Value->AsNumber()));
	}
	case EJson::Boolean:
	{
		return FEJsonValue::MakeBoolean(Value->AsBool());
	}
	case EJson::Array:
	{
		const TArray<TSharedPtr<FJsonValue>>& JsonArray = Value->AsArray();
		auto NewJsonArray = MakeShared<FEJsonValue>();
		NewJsonArray->JsonType = EEJsonType::JSON_ARRAY;
		NewJsonArray->ValueArray.AddZeroed(JsonArray.Num());
		for (int i = 0; i < JsonArray.Num(); i++) {
			NewJsonArray->ValueArray[i] = FEJsonValue::MakeFromCPPVersion(JsonArray[i]);
		}

		return NewJsonArray;
	}
	case EJson::Object:
	{
		const TSharedPtr<FJsonObject>& JsonObject = Value->AsObject();
		auto NewJsonObject = MakeShared<FEJsonValue>();
		NewJsonObject->JsonType = EEJsonType::JSON_OBJECT;

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : JsonObject->Values) {
			NewJsonObject->ValueObject.Add(Pair.Key, FEJsonValue::MakeFromCPPVersion(Pair.Value));
		}

		return NewJsonObject;
	}
	}

	return nullptr;
}

TSharedPtr<FEJsonValue> FEJsonValue::MakeFromString(const FString& Value) {
	auto ParsedJson = HelperParseJSON_c(Value);
	if (!ParsedJson.IsValid()) {
		return nullptr;
	}

	return FEJsonValue::MakeFromCPPVersion(ParsedJson);
}

bool FEJsonValue::SetFieldValue(const FString& Field, const TSharedPtr<FEJsonValue> Value) {
	if (this->JsonType != EEJsonType::JSON_OBJECT) {
		return false;
	}

	this->ValueObject.Emplace(Field, TSharedPtr<FEJsonValue>(Value));
	return true;
}

bool FEJsonValue::SetFieldString(const FString& Field, const FString& Value) {
	return this->SetFieldValue(Field, FEJsonValue::MakeString(Value));
}

bool FEJsonValue::SetFieldNumber(const FString& Field, float Value) {
	return this->SetFieldValue(Field, FEJsonValue::MakeNumber(Value));
}

bool FEJsonValue::SetFieldBoolean(const FString& Field, bool Value) {
	return this->SetFieldValue(Field, FEJsonValue::MakeBoolean(Value));
}

bool FEJsonValue::SetFieldNull(const FString& Field) {
	return this->SetFieldValue(Field, FEJsonValue::MakeNull());
}

bool FEJsonValue::SetFieldArray(const FString& Field, const TArray<TSharedPtr<FEJsonValue>>& Value) {
	return this->SetFieldValue(Field, FEJsonValue::MakeArray(Value));
}

bool FEJsonValue::SetFieldObject(const FString& Field, const TMap<FString, TSharedPtr<FEJsonValue>>& Value) {
	return this->SetFieldValue(Field, FEJsonValue::MakeObject(Value));
}


TSharedPtr<FEJsonValue> FEJsonValue::GetFieldValue(const FString& Field) const {
	if (this->JsonType != EEJsonType::JSON_OBJECT) {
		return nullptr;
	}

	auto Value = this->ValueObject.Find(Field);
	if (Value == nullptr) {
		return nullptr;
	}

	return *Value;
}

bool FEJsonValue::GetFieldString(const FString& Field, FString& Value) const {
	auto RawValue = this->GetFieldValue(Field);
	if (RawValue == nullptr) {
		return false;
	}

	return RawValue->GetString(Value);
}


bool FEJsonValue::GetFieldNumber(const FString& Field, float& Value) const {
	auto RawValue = this->GetFieldValue(Field);
	if (RawValue == nullptr) {
		return false;
	}

	return RawValue->GetNumber(Value);
}

bool FEJsonValue::GetFieldBoolean(const FString& Field, bool& Value) const {
	auto RawValue = this->GetFieldValue(Field);
	if (RawValue == nullptr) {
		return false;
	}

	return RawValue->GetBoolean(Value);
}

bool FEJsonValue::GetFieldArray(const FString& Field, TArray<TSharedPtr<FEJsonValue>>& Value) const {
	auto RawValue = this->GetFieldValue(Field);
	if (RawValue == nullptr) {
		return false;
	}

	return RawValue->GetArray(Value);
}

bool FEJsonValue::GetFieldObject(const FString& Field, TMap<FString, TSharedPtr<FEJsonValue>>& Value)  const {
	auto RawValue = this->GetFieldValue(Field);
	if (RawValue == nullptr) {
		return false;
	}

	return RawValue->GetObject(Value);
}


TSharedPtr<FJsonValue> FEJsonValue::ToCPPVersion() const {
	switch (this->JsonType) {
	case EEJsonType::JSON_NONE:
	{
		return nullptr;
	}
	case EEJsonType::JSON_NULL:
	{
		return MakeShared<FJsonValueNull>();
	}
	case EEJsonType::JSON_STRING:
	{
		return MakeShared<FJsonValueString>(this->ValueString);
	}
	case EEJsonType::JSON_NUMBER:
	{
		return MakeShared<FJsonValueNumber>(this->ValueNumber);
	}
	case EEJsonType::JSON_BOOLEAN:
	{
		return MakeShared<FJsonValueBoolean>(this->ValueBool);
	}
	case EEJsonType::JSON_ARRAY:
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		for (const TSharedPtr<FEJsonValue>& Value : this->ValueArray) {
			if (Value != nullptr) {
				JsonArray.Emplace(Value->ToCPPVersion());
			} else {
				JsonArray.Emplace(nullptr);
			}
		}

		return MakeShared<FJsonValueArray>(JsonArray);
	}
	case EEJsonType::JSON_OBJECT:
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		for (const TPair<FString, TSharedPtr<FEJsonValue>>& Pair : this->ValueObject) {
			if (Pair.Value != nullptr) {
				JsonObject->Values.Emplace(Pair.Key, Pair.Value->ToCPPVersion());
			} else {
				JsonObject->Values.Emplace(Pair.Key, nullptr);
			}
		}

		return MakeShared<FJsonValueObject>(JsonObject);
	}
	}

	return nullptr;
}

FString FEJsonValue::ToString(bool bPretty) const {
	auto JsonValue = this->ToCPPVersion();

	if (!JsonValue.IsValid()) {
		return FString();
	}

	return HelperStringifyJSON_c(JsonValue, bPretty);
}

EEJsonType FEJsonValue::GetJsonType() const {
	return this->JsonType;
}

void FEJsonValue::SetJsonType(EEJsonType Type) {
	this->JsonType = Type;
}

TSharedPtr<FJsonValue> HelperParseJSON_c(const FString& RawJson) {

	FString TmpRawJson;

	TmpRawJson.Reserve(RawJson.Len() + 2);
	TmpRawJson += TEXT("[");
	TmpRawJson += RawJson;
	TmpRawJson += TEXT("]");


	TArray<TSharedPtr<FJsonValue>> ResultArray;
	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(TmpRawJson);
	
	if (!FJsonSerializer::Deserialize(JsonReader, ResultArray) || ResultArray.Num() == 0) {
		return nullptr;
	}
	
	return ResultArray[0];
}

FString HelperStringifyJSON_c(TSharedPtr<FJsonValue> JsonValue, bool bPretty) {
	check(JsonValue.IsValid());

	FString RawJson;
	if (bPretty) {
		auto JsonWriter =  TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&RawJson);
		if (!FJsonSerializer::Serialize(JsonValue, FString(), JsonWriter)) {
		}
	} else {
		auto JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&RawJson);
		if (!FJsonSerializer::Serialize(JsonValue, FString(), JsonWriter)) {
		}
	}

	if (RawJson.Len() > 0 && RawJson[0] == TEXT(',')) {
		if (bPretty) {
			RawJson.RemoveAt(0, 3, false);
		} else {
			RawJson.RemoveAt(0, 1, false);
		}
	}

	return RawJson;
}