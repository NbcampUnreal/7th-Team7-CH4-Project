#include "VGLobbyPhase.h"
#include "Core/VGGameMode.h"
#include "Common/VGGameplayTags.h"

void UVGLobbyPhase::EnterPhase()
{
	Super::EnterPhase();
	
	UE_LOG(LogTemp, Warning, TEXT("[VGLobbyPhase] 로비 페이즈 진입"));
}

void UVGLobbyPhase::ExitPhase()
{
	UE_LOG(LogTemp, Warning, TEXT("[VGLobbyPhase] 전원 레디 완료! 로비 페이즈 종료"));
	Super::ExitPhase();
}

void UVGLobbyPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->NotifyPhaseCompleted(this);
	}
}

bool UVGLobbyPhase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
	return true;
}

bool UVGLobbyPhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	return true;
}

bool UVGLobbyPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	return false;
}
