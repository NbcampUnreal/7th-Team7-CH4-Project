#include "VGMissionPhase.h"
#include "Core/VGGameMode.h"
#include "TimerManager.h"
#include "Character/VGCharacterBase.h"

void UVGMissionPhase::EnterPhase()
{
	Super::EnterPhase();
	UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈가 시작"));
	
	if (AVGGameState* VGGameState = GameModeRef->GetWorld()->GetGameState<AVGGameState>())
	{
		// 서버 기준 시작 시간 저장
		VGGameState->PhaseStartTime = VGGameState->GetServerWorldTimeSeconds();
		// 서버 기준 페이즈 끝 시간 저장
		VGGameState->PhaseEndTime = VGGameState->PhaseStartTime + PhaseDuration;
	}
	
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
	if (GameModeRef)
	{
		GameModeRef->NotifyPhaseCompleted(this);
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
	// 상호작용 불가능한 물건이거나 자기 자신이면 상호작용 X
	if (!InteractableObject || InteractableObject == Player)
	{
		return false;
	}
	
	return true;
}

bool UVGMissionPhase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
	return true;
}

bool UVGMissionPhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
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
		
		float RemainingTime = VGGameState->GetRemainingPhaseTime();
		
		GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		
		if (RemainingTime <= 0.0f)
		{
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
