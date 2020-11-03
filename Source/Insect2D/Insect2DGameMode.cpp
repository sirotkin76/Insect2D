// Copyright Epic Games, Inc. All Rights Reserved.

#include "Insect2DGameMode.h"
#include "Insect2DCharacter.h"

AInsect2DGameMode::AInsect2DGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AInsect2DCharacter::StaticClass();	
}
