// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoCPPHeroCharacter.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "DemoCPPPlayerController.h"

ADemoCPPHeroCharacter::ADemoCPPHeroCharacter() {

	this->SetTickableWhenPaused(true);
	
	{
		// ======= capsule 组件 =======
		auto CapsuleComp = this->GetCapsuleComponent();
		CapsuleComp->SetCapsuleHalfHeight(75.f);
		CapsuleComp->SetCapsuleRadius(1.f);
	}

	
	{
		// ======= mesh 组件 =======
		// 1. skeletal
		auto MeshComp = this->GetMesh();
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("SkeletalMesh'/Game/Blender/mc3.mc3_Body7'"));
		if (MeshFinder.Succeeded()) {
			MeshComp->SetSkeletalMesh(MeshFinder.Object);
			UE_LOG(LogTopDownSoulsLike, Log, TEXT("[%s:%d] set skeletal mesh: ok"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		} else {
			UE_LOG(LogTopDownSoulsLike, Error, TEXT("[%s:%d] set skeletal mesh: failed"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		}

		// 2. transform
		FTransform MeshTransform = FTransform(FRotator(0), FVector(0, 0, -80), FVector(0.2));
		MeshComp->SetRelativeTransform(MeshTransform);

		// 3. anim instance
		static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceFinder(TEXT("/Game/Blueprint/DemoCPP/ABP_DemoCPP"));
		if (AnimInstanceFinder.Succeeded()) {
			MeshComp->SetAnimInstanceClass(AnimInstanceFinder.Class);
			UE_LOG(LogTopDownSoulsLike, Log, TEXT("[%s:%d] set anim instance: ok"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		} else {
			UE_LOG(LogTopDownSoulsLike, Error, TEXT("[%s:%d] set anim instance: failed"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		}

		//  4. material
		static ConstructorHelpers::FObjectFinder<UMaterial> HeroMaterialFinder(TEXT("Material'/Game/Materials/M_Hero.M_Hero'"));
		if (HeroMaterialFinder.Succeeded()) {
			MeshComp->SetMaterial(0, HeroMaterialFinder.Object);
			UE_LOG(LogTopDownSoulsLike, Log, TEXT("[%s:%d] set material: ok"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		} else {
			UE_LOG(LogTopDownSoulsLike, Error, TEXT("[%s:%d] set material: failed"), ANSI_TO_TCHAR(__FILE__), __LINE__);
		}
	}

	
	{
		// ======= spring arm 组件 =======
		this->SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm1"));
		this->SpringArmComp->SetupAttachment(this->RootComponent);
		this->SpringArmComp->TargetArmLength = 1000.f;
		this->SpringArmComp->bDoCollisionTest = false;
		this->SpringArmComp->bEnableCameraLag = true;
		this->SpringArmComp->CameraLagSpeed = 20.f;
		this->SpringArmComp->SetAbsolute(false, true, false);
		FTransform SpringArmCompTransform = FTransform(FRotator(-45.f, 0.f, 0.f));
		this->SpringArmComp->SetRelativeTransform(SpringArmCompTransform);
	}

	{
		// ======== camera 组件 ========
		this->CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera1"));
		this->CameraComp->SetupAttachment(this->SpringArmComp);
	}

	{
		// ======= movement 组件 ========
		this->GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
		this->GetCharacterMovement()->MaxWalkSpeed = 400.f;
	}
}

void ADemoCPPHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (this->CachedPlayerController == nullptr ||PlayerInputComponent == nullptr) {
		return;
	}

	PlayerInputComponent->BindAxis("GoForward", this, &ADemoCPPHeroCharacter::DoGoForwardBack);
	PlayerInputComponent->BindAxis("GoBack", this, &ADemoCPPHeroCharacter::DoGoForwardBack);
	PlayerInputComponent->BindAxis("GoLeft", this, &ADemoCPPHeroCharacter::DoGoLeftRight);
	PlayerInputComponent->BindAxis("GoRight", this, &ADemoCPPHeroCharacter::DoGoLeftRight);
}

void ADemoCPPHeroCharacter::PossessedBy(AController* NewController) {
	this->CachedPlayerController = Cast<ADemoCPPPlayerController>(NewController);

}

void ADemoCPPHeroCharacter::DoGoForwardBack(float Value) {
	if (this->CachedPlayerController == nullptr || Value == 0.f) {
		return;
	}

	this->AddMovementInput(FVector::ForwardVector, Value);
}

void ADemoCPPHeroCharacter::DoGoLeftRight(float Value) {
	if (this->CachedPlayerController == nullptr || Value == 0.f) {
		return;
	}

	this->AddMovementInput(FVector::RightVector, Value);
}
