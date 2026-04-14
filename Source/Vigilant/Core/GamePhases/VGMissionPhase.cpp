#include "VGMissionPhase.h"
#include "Core/VGGameMode.h"
#include "TimerManager.h"
#include "Character/VGCharacterBase.h"

void UVGMissionPhase::EnterPhase()
{
	Super::EnterPhase();
	UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈가 시작"));
	
	// 테스트용 타이머
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().SetTimer(
			PhaseTimerHandle,
			this,
			&UVGMissionPhase::OnMissionTimeUp,
			PhaseDuration, 
			false);
	}
}

void UVGMissionPhase::ExitPhase()
{
	
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	}
	
	Super::ExitPhase();
}

void UVGMissionPhase::ExecutePhaseResult()
{
	if (GameModeRef && NextPhaseClass)
	{
		GameModeRef->TransitionToPhase(NextPhaseClass);
	}
}
void UVGMissionPhase::PausePhase()
{
	Super::PausePhase();
	
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().PauseTimer(PhaseTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈 타이머 일시정지"));
	}
}

void UVGMissionPhase::ResumePhase()
{
	Super::ResumePhase();
	
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().UnPauseTimer(PhaseTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈 타이머 재개"));
	}	
}

bool UVGMissionPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	if (!InteractableObject || InteractableObject == Player)
	{
		return false;
	}
	
	if (InteractableObject->IsA<AVGCharacterBase>())
	{
		return false;
	}
	
	return true;
}

void UVGMissionPhase::OnMissionCleared(float TimeReducedAmount)
{
	Super::OnMissionCleared(TimeReducedAmount);
	
	if (AVGGameState* VGGameState = GameModeRef->GetWorld()->GetGameState<AVGGameState>())
	{
		VGGameState->PhaseEndTime -= TimeReducedAmount;
		
		float ElapsedTime = PhaseDuration - VGGameState->GetRemainingPhaseTime();
		VGGameState->BossNerfRate = FMath::Clamp(ElapsedTime / PhaseDuration, 0.1f, 1.0f);
		
		if (!GameModeRef) return;

		float RemainingTime = VGGameState->GetRemainingPhaseTime();
		
		if (RemainingTime <= 0.0f)
		{
			GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
			OnMissionTimeUp();
		}
		else
		{
			GameModeRef->GetWorldTimerManager().SetTimer(
				PhaseTimerHandle,
				this,
				&UVGMissionPhase::OnMissionTimeUp,
				RemainingTime,
				false);
		}
	}
}

void UVGMissionPhase::OnMissionTimeUp()
{
	UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈 시간 종료"));
	ExecutePhaseResult();
}
