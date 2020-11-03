// Copyright Epic Games, Inc. All Rights Reserved.

#include "Insect2DCharacter.h"


#include "AIController.h"
#include "ModuleDescriptor.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Containers/UnrealString.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// AInsect2DCharacter

AInsect2DCharacter::AInsect2DCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->SetUsingAbsoluteRotation(true);
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////
// Animation

void AInsect2DCharacter::UpdateAnimation( UPaperFlipbook* NewAnim )
{
	if( GetSprite()->GetFlipbook() != NewAnim 	)
	{
		GetSprite()->SetFlipbook(NewAnim);
	}
}


void AInsect2DCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateCharacter();	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AInsect2DCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AInsect2DCharacter::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AInsect2DCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AInsect2DCharacter::TouchStopped);
}

void AInsect2DCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

void AInsect2DCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void AInsect2DCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void AInsect2DCharacter::UpdateCharacter()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();
	const bool bFooling = GetMovementComponent()->IsFalling();
	
	float TravelDirection = PlayerVelocity.X;
	ECharacterStatus ConditionCharacterStatus;
	
	if (bFooling)
	{
		Idle();
		ConditionCharacterStatus = ECharacterStatus::Fall;
	} else
	{
		if (PlayerSpeedSqr > 0.1f)
		{
			ConditionCharacterStatus = ECharacterStatus::Run;
		} else
			if (RunLogic == ERunLogic::RunNone || CharacterStatus == ECharacterStatus::Run ) ConditionCharacterStatus = ECharacterStatus::Idle;

		//ConditionCharacterStatus = (PlayerSpeedSqr > 0.1f)? ECharacterStatus::Run : ECharacterStatus::Idle ;
		
		if (ConditionCharacterStatus != CharacterStatus)
		{
			CharacterStatus = ConditionCharacterStatus;

			switch(CharacterStatus)
			{
			case 0:
				Idle();
				//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("IDLEIDLEIDLEIDLE"));
				break;
			case 1:
				Running();
				//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("RunningRunningRunningRunning"));
				break;
			default:
				//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("defaultdefaultdefault"));
				break;
				
			}
		} //else GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("elseelseelse"));
		// Are we moving or standing still?
		//DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	}

	// Set the rotation so that the character faces his direction of travel.
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}


void AInsect2DCharacter::Idle()
{
	UpdateAnimation(IdleAnimation);
}


void AInsect2DCharacter::Running()
{
	float fRamecount = GetSprite()->GetFlipbookLengthInFrames() + 1;
	const float fFlipbookLength = GetSprite()->GetFlipbookLength();
	const float fTimeAnimation = fFlipbookLength / fRamecount;

	
	FTimerHandle TimerHandle;
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();
	const bool bFooling = GetMovementComponent()->IsFalling();
	
	//FString Info = FString::Printf(TEXT("time - %f"), fTimeAnimation);
	
	//if (PlayerSpeedSqr > 0.1f)
	//if (!bFooling ) {
		
		switch(RunLogic)
		{
			case 0:
				if (PlayerSpeedSqr == 0.0f) break;
				// RunNone
				RunLogic = ERunLogic::PreRun;
				UpdateAnimation(PreRunningAnimation);
				GetWorldTimerManager().SetTimer(TimerHandle, this, &AInsect2DCharacter::Running, 1.0f, false, fTimeAnimation);	
				//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Blue, FString::Printf(TEXT("time - %f"), fTimeAnimation));
			break;
			case 1:
				//PreRun
				(PlayerSpeedSqr > 0.5f) ? RunLogic = ERunLogic::Running : RunLogic = ERunLogic::RunStop;
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("2"));
				Running();
			break;
			case 2:
				//Running
				if (PlayerSpeedSqr > 0.5f)
				{
					UpdateAnimation(RunningAnimation);
					//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("RunningAnimationRunningAnimationRunningAnimation"));
					GetWorldTimerManager().SetTimer(TimerHandle, this, &AInsect2DCharacter::Running, 1.0f, false, fTimeAnimation);	
				}  else
				{
					RunLogic = ERunLogic::RunStop;
					UpdateAnimation(StopRunningAnimation);
					GetWorldTimerManager().SetTimer(TimerHandle, this, &AInsect2DCharacter::Running, 1.0f, false, fTimeAnimation);
					//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Black, FString::Printf(TEXT("timetimetimetimetime - %f"), fTimeAnimation)) ;
				};
			break;
			case 3:
				//RunStop
				RunLogic = ERunLogic::RunNone;
				Idle();
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("5"));
				GetWorldTimerManager().ClearTimer(TimerHandle);
			break;
		}
	//} else
	//{
		//GetWorldTimerManager().ClearTimer(TimerHandle);
		//ERunLogic = ERunLogic::RunNone;
		//Idle();
	//}
		


	
}