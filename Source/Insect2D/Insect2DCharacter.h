// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "Insect2DCharacter.generated.h"

UENUM(BlueprintType)
enum class  ECharacterStatus : uint8
{
	Idle,
	Run,
	Fall,
	Attack,
	Block,
};

UENUM(BlueprintType)
enum class  ERunLogic : uint8
{
	RunNone,
	PreRun,
	Running,
	RunStop
};

class UTextRenderComponent;

/**
 * This class is the default character for Insect2D, and it is responsible for all
 * physical interaction between the player and the world.
 *
 * The capsule component (inherited from ACharacter) handles collision with the world
 * The CharacterMovementComponent (inherited from ACharacter) handles movement of the collision capsule
 * The Sprite component (inherited from APaperCharacter) handles the visuals
 */
UCLASS(config=Game)
class AInsect2DCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UTextRenderComponent* TextComponent;
	virtual void Tick(float DeltaSeconds) override;
protected:
	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animations)
	class UPaperFlipbook* RunningAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	class UPaperFlipbook* PreRunningAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	class UPaperFlipbook* StopRunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation(UPaperFlipbook* NewAnim);

	/** Called for side to side input */
	void MoveRight(float Value);

	void UpdateCharacter();


	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	void Idle();
	
	void Running();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animations")
	ERunLogic RunLogic;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animations")
	ECharacterStatus CharacterStatus = ECharacterStatus::Idle;

public:
	AInsect2DCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};

