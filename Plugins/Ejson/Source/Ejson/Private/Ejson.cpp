// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ejson.h"
#include "Json.h"
#include "Ejson_c.h"
	




#define LOCTEXT_NAMESPACE "FEjsonModule"



void FEjsonModule::StartupModule()
{
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UEjsonValue::TestAll();
}

void FEjsonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEjsonModule, Ejson)

UEjsonValue::UEjsonValue(){
	this->Raw = MakeShared<FEJsonValue>();
}


#if UE_BUILD_SHIPPING
void UEjsonValue::TestAll() {

}
#else
void UEjsonValue::TestAll() {
#pragma region deserialize test
	{
		auto TestTarget = UEjsonValue::MakeJsonFromString(R"({"name":"dan","list":["d","dd","ddd"], "role":{"level":1,"color":"red","info":{"groupid":1001}}})");
		auto Tmp_1 = TestTarget->GetFieldValue("list");

		ensureAlways(Tmp_1 != nullptr);

		TArray<UEjsonValue*> Tmp_2;
		ensureAlways(Tmp_1->JsonAsArray(Tmp_2) == true);
		
		Tmp_2[0]->SetJsonString("c");

		
		ensureAlways(Tmp_1->ToString(false) ==  R"(["c","dd","ddd"])");

		auto Tmp_3 = TestTarget->GetFieldValue("role");
		ensureAlways(Tmp_3 != nullptr);
		
		auto Tmp_4 = Tmp_3->GetFieldValue("level");
		float Tmp_5;
		ensureAlways(Tmp_4->JsonAsNumber(Tmp_5) == true);
		ensureAlways(int32(Tmp_5) == int32(1));
	
		Tmp_3->SetFieldValue("level", UEjsonValue::MakeJsonNumber(37));

		auto Tmp_7 = Tmp_3->GetFieldValue("color");
		FString Tmp_8;
		ensureAlways(Tmp_7->JsonAsString(Tmp_8) == true);
		ensureAlways(Tmp_8 == "red");
		Tmp_7->SetJsonString("blue");
		ensureAlways(Tmp_7->JsonAsString(Tmp_8) == true);
		ensureAlways(Tmp_8 == "blue");

		auto Tmp_9 = Tmp_3->GetFieldValue("info");
		TMap<FString, UEjsonValue*> Tmp_10;
		ensureAlways(Tmp_9->JsonAsObject(Tmp_10) == true);
		ensureAlways(Tmp_10.Contains("groupid"));


		ensureAlways(TestTarget->ToString(false) == R"({"name":"dan","list":["c","dd","ddd"],"role":{"level":37,"color":"blue","info":{"groupid":1001}}})");
	}
	
	
#pragma endregion
	{
		TMap<FString, UEjsonValue*> Tmp_1;
		Tmp_1.Emplace("name", UEjsonValue::MakeJsonString("dan"));
		Tmp_1.Emplace("age", UEjsonValue::MakeJsonNumber(37));
		TMap<FString, UEjsonValue*> Tmp_2;
		Tmp_2.Emplace("level", UEjsonValue::MakeJsonNumber(999));
		Tmp_1.Emplace("info", UEjsonValue::MakeJsonObject(Tmp_2));
		auto Tmp_3 = UEjsonValue::MakeJsonObject(Tmp_1);

		//UE_LOG(LogTemp, Log, TEXT("%s"), *(Tmp_3->ToString(false)));
		ensureAlways(Tmp_3->ToString(false) == R"({"name":"dan","age":37,"info":{"level":999}})");
	}

#pragma region serialize test
	

#pragma endregion
	
	UE_LOG(LogTemp, Log, TEXT("Ejson module: all tests done!"));
}
#endif

void UEjsonValue::Reset() {
	if (!this->Raw.IsValid()) {
		return;
	}
	
	this->Raw->Reset();
}

bool UEjsonValue::JsonAsString(FString& Value) const {
	if (!this->Raw.IsValid()) {
		return false;
	}

	return this->Raw->GetString(Value);
}

bool UEjsonValue::JsonAsBoolean(bool& Value) const {
	if (!this->Raw.IsValid()) {
		return false;
	}

	return this->Raw->GetBoolean(Value);
}

bool UEjsonValue::JsonAsNumber(float& Value) const {
	if (!this->Raw.IsValid()) {
		return false;
	}

	return this->Raw->GetNumber(Value);
}

bool UEjsonValue::JsonAsArray(TArray<UEjsonValue*>& Value) const{	
	if (!this->Raw.IsValid()) {
		return false;
	}

	if (this->Raw->GetJsonType() != EEJsonType::JSON_ARRAY) {
		return false;
	}

	Value = this->ValueArray;
	return true;
	
}

bool UEjsonValue::JsonAsObject(TMap<FString, UEjsonValue*>& Value) const {
	if (!this->Raw.IsValid()) {
		return false;
	}

	if (this->Raw->GetJsonType() != EEJsonType::JSON_OBJECT) {
		return false;
	}


	Value = this->ValueObject;
	return true;
}

bool UEjsonValue::SetJsonString(const FString& Value) {
	return this->Raw->SetString(Value);
}

bool UEjsonValue::SetJsonBoolean(bool Value) {
	return this->Raw->SetBoolean(Value);
}

bool UEjsonValue::SetJsonNumber(float Value) {
	return this->Raw->SetNumber(Value);
}


bool UEjsonValue::SetJsonArray(const TArray<UEjsonValue*>& Values) {

	TArray<TSharedPtr<FEJsonValue>> TmpArray;
	TmpArray.AddZeroed(Values.Num());

	for (int i = 0; i < Values.Num(); i++) {
		TmpArray[i] = Values[i]->Raw;
	}

	if (!this->Raw->SetArray(TmpArray)) {
		return false;
	}

	this->ValueArray = Values;
	return true;
}

bool UEjsonValue::SetJsonObject(const TMap<FString, UEjsonValue*>& Values) {

	TMap<FString, TSharedPtr<FEJsonValue>> TmpMap;

	for (const TPair<FString, UEjsonValue*>& Pair : Values) {
		TmpMap.Emplace(Pair.Key, Pair.Value->Raw);
	}

	if (!this->Raw->SetObject(TmpMap)) {
		return false;
	}

	this->ValueObject = Values;
	return true;
}

UEjsonValue* UEjsonValue::MakeJsonString(const FString& Value) {
	auto NewJsonValue = NewObject<UEjsonValue>();

	NewJsonValue->Raw = FEJsonValue::MakeString(Value);
	
	return NewJsonValue;
}

UEjsonValue* UEjsonValue::MakeJsonNumber(float Value) {
	auto NewJsonValue = NewObject<UEjsonValue>();

	NewJsonValue->Raw = FEJsonValue::MakeNumber(Value);
	
	return NewJsonValue;
}

UEjsonValue* UEjsonValue::MakeJsonBoolean(bool Value) {
	auto NewJsonValue = NewObject<UEjsonValue>();

	NewJsonValue->Raw = FEJsonValue::MakeBoolean(Value);

	return NewJsonValue;
}


UEjsonValue* UEjsonValue::MakeJsonNull() {
	auto NewJsonValue = NewObject<UEjsonValue>();

	NewJsonValue->Raw = FEJsonValue::MakeNull();

	return NewJsonValue;
}

UEjsonValue* UEjsonValue::MakeJsonArray(const TArray<UEjsonValue*>& Values) {
	auto NewJsonValue = NewObject<UEjsonValue>();

	TArray<TSharedPtr<FEJsonValue>> TmpArray;
	TmpArray.AddZeroed(Values.Num());

	for (int i = 0; i < Values.Num(); i++) {
		TmpArray[i] = Values[i]->Raw;
	}

	NewJsonValue->Raw = FEJsonValue::MakeArray(TmpArray);
	NewJsonValue->ValueArray = Values;

	
	return NewJsonValue;
}

UEjsonValue* UEjsonValue::MakeJsonObject(const TMap<FString, UEjsonValue*>& Value) {
	auto NewJsonValue = NewObject<UEjsonValue>();

	TMap<FString, TSharedPtr<FEJsonValue>> TmpMap;

	for (const TPair<FString, UEjsonValue*>& Pair : Value) {
		TmpMap.Emplace(Pair.Key, Pair.Value->Raw);
	}
	
	NewJsonValue->Raw = FEJsonValue::MakeObject(TmpMap);
	NewJsonValue->ValueObject = Value;

	return NewJsonValue;
}

UEjsonValue* UEjsonValue::MakeFromCPPVersion(TSharedPtr<FJsonValue> Value) {
	if (!Value.IsValid()) {
		return nullptr;
	}

	auto TmpRawJsonValue = FEJsonValue::MakeFromCPPVersion(Value);

	if (!TmpRawJsonValue.IsValid()) {
		return nullptr;
	}

	auto TmpJsonValue = NewObject<UEjsonValue>();

	TmpJsonValue->MakeFromCPPVersionInternal(TmpRawJsonValue);

	return TmpJsonValue;
}

UEjsonValue* UEjsonValue::MakeJsonFromString(const FString& Value) {
	auto ParsedJson = HelperParseJSON(Value);
	if (!ParsedJson.IsValid()) {
		return nullptr;
	}

	return UEjsonValue::MakeFromCPPVersion(ParsedJson);
}

bool UEjsonValue::SetFieldValue(const FString& Field, const UEjsonValue* Value) {
	if (!this->Raw.IsValid()) {
		return false;
	}

	if (!this->Raw->SetFieldValue(Field, Value->Raw)) {
		return false;
	}

	this->ValueObject.Emplace(Field, const_cast<UEjsonValue*>(Value));
	return true;
}


UEjsonValue* UEjsonValue::GetFieldValue(const FString& Field) const {
	if (!this->Raw.IsValid()) {
		return nullptr;
	}

	auto Value = this->ValueObject.Find(Field);
	if (Value == nullptr) {
		return nullptr;
	}

	return *Value;
}


TSharedPtr<FJsonValue> UEjsonValue::ToCPPVersion() const {
	if (!this->Raw.IsValid()) {
		return nullptr;
	}

	return this->Raw->ToCPPVersion();
}

FString UEjsonValue::ToString(bool bPretty) const {
	if (!this->Raw.IsValid()) {
		return FString();
	}

	return this->Raw->ToString(bPretty);
}


UEjsonValue* UEjsonValue::MakeFromCPPVersionInternal(TSharedPtr<FEJsonValue> Value) {
	check(Value.IsValid());
	
	this->Raw = Value;

	// make cahce for array and object
	if (this->Raw->GetJsonType() == EEJsonType::JSON_ARRAY) {
	
		this->ValueArray.AddZeroed(this->Raw->ValueArray.Num());

		for (int i = 0; i < this->Raw->ValueArray.Num(); i++) {
			auto TmpObj = NewObject<UEjsonValue>();
			this->ValueArray[i] = TmpObj->MakeFromCPPVersionInternal(this->Raw->ValueArray[i]);
		}
	}else if (this->Raw->GetJsonType() == EEJsonType::JSON_OBJECT) {

		for (const TPair<FString, TSharedPtr<FEJsonValue>>& Pair : this->Raw->ValueObject) {
			auto TmpObj = NewObject<UEjsonValue>();
			this->ValueObject.Emplace(Pair.Key, TmpObj->MakeFromCPPVersionInternal(Pair.Value));
		}
	} else {
		// pass


		
	}

	return this;

}


TSharedPtr<FJsonValue> HelperParseJSON(const FString& RawJson) {
	return HelperParseJSON_c(RawJson);
}

FString HelperStringifyJSON(TSharedPtr<FJsonValue> JsonValue, bool bPretty) {
	return HelperStringifyJSON_c(JsonValue, bPretty);
}