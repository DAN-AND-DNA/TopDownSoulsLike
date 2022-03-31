// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoCPPPlayerController.h"
#include "DemoCPPHeroCharacter.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "TopDownSoulsLike/TopDownSoulsLike.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

ADemoCPPPlayerController::ADemoCPPPlayerController() 
		: InputMode() {
	this->bShowMouseCursor = true;
	this->bEnableClickEvents = true;
	this->InputMode.SetHideCursorDuringCapture(false);
}

void ADemoCPPPlayerController::BeginPlay() {
	Super::BeginPlay();

	this->SetInputMode(this->InputMode);
}

void ADemoCPPPlayerController::OnPossess(APawn* NewPawn) {
	Super::OnPossess(NewPawn);

	this->CachedHero = Cast<ADemoCPPHeroCharacter>(NewPawn);
}


void ADemoCPPPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	this->SetMouseDirection();
	this->FaceMouseDirection(DeltaTime);
}

void ADemoCPPPlayerController::SetMouseDirection() {

	FHitResult Hit;
	
	this->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, Hit);

	if (!Hit.bBlockingHit) {
		return;
	}


	auto CachedHeroLocation = this->CachedHero->GetActorLocation() * FVector(1,1,0);
	auto HitLocation = Hit.Location * FVector(1, 1, 0);

	this->MouseDirection =  UKismetMathLibrary::FindLookAtRotation(CachedHeroLocation, HitLocation);	
}


void ADemoCPPPlayerController::FaceMouseDirection(float DeltaTime) {
	auto CachedHeroRotation = this->CachedHero->GetActorRotation();
	
	this->SetControlRotation(UKismetMathLibrary::RInterpTo(CachedHeroRotation, this->MouseDirection, DeltaTime, 10.f));
}
