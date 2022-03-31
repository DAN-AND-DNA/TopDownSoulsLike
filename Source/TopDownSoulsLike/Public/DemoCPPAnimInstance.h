// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DemoCPPAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWNSOULSLIKE_API UDemoCPPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UDemoCPPAnimInstance();

	UFUNCTION(BlueprintCallable, Category=DemoCPP)
	void UpdateSpeedAndDirection();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=DemoCPP)
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=DemoCPP)
	float Direction;
};
