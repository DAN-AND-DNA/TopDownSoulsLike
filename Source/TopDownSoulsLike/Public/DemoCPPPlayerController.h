// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DemoCPPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWNSOULSLIKE_API ADemoCPPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADemoCPPPlayerController();

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* Pawn) override;
	virtual void Tick(float DeltaTime) override;

	void SetMouseDirection();
	void FaceMouseDirection(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DemoCPP)
	class ADemoCPPHeroCharacter* CachedHero;

private:
	FRotator MouseDirection;
	FInputModeGameAndUI InputMode;

};
