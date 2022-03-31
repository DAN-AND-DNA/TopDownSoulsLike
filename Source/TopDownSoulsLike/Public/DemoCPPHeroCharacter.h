// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TopDownSoulsLike/TopDownSoulsLike.h"
#include "DemoCPPCharacter.h"
#include "DemoCPPHeroCharacter.generated.h"



/**
 * 
 */
UCLASS()
class TOPDOWNSOULSLIKE_API ADemoCPPHeroCharacter : public ADemoCPPCharacter
{
	GENERATED_BODY()
public:
	ADemoCPPHeroCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
protected:
	virtual void PossessedBy(AController* NewController) override;

	void DoGoForwardBack(float Value);
	//void DoGoBack(float Value);
	void DoGoLeftRight(float Value);
	//void DoGoRight(float Value);
protected:
	class USpringArmComponent* SpringArmComp;
	class UCameraComponent* CameraComp;

private:
	class ADemoCPPPlayerController* CachedPlayerController;


};
