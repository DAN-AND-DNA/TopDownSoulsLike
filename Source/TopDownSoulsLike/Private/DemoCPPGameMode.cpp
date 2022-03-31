// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoCPPGameMode.h"
#include "DemoCPPHeroCharacter.h"
#include "DemoCPPPlayerController.h"


ADemoCPPGameMode::ADemoCPPGameMode() {
	this->PlayerControllerClass = ADemoCPPPlayerController::StaticClass();
	this->DefaultPawnClass = ADemoCPPHeroCharacter::StaticClass();
}