// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoCPPAnimInstance.h"
#include "Math/Vector.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

UDemoCPPAnimInstance::UDemoCPPAnimInstance()
	: Speed(0.f)
	, Direction(0.f) {
}

void UDemoCPPAnimInstance::UpdateSpeedAndDirection(){
	const APawn* OwningPawn = this->TryGetPawnOwner();

	if (OwningPawn == nullptr) {
		return;
	}

	FVector Velocity = OwningPawn->GetVelocity();
	float VelocityLength = Velocity.Size();

	if (VelocityLength == 0) {
		this->Speed = 0.f;
		this->Direction = 0.f;
	} else {
		FVector ForwardVector = OwningPawn->GetActorForwardVector();
		FVector RightVector = OwningPawn->GetActorRightVector();

		this->Speed = UKismetMathLibrary::Round(FVector::DotProduct(Velocity, ForwardVector) / VelocityLength);
		this->Direction = UKismetMathLibrary::Round(FVector::DotProduct(Velocity, RightVector)/ VelocityLength);
	}
}
