#include "VGMissionPhase.h"
#include "Core/VGGameMode.h"
#include "TimerManager.h"

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
			15.0f,
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

void UVGMissionPhase::OnMissionCleared(int32 TimeReducedAmount)
{
	bool bIsAllMissionDone = true; 
    
	if (bIsAllMissionDone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 모든 미션 조기 달성! 다음 페이즈로 넘어갑니다."));
		
		GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		ExecutePhaseResult();
	}
}

void UVGMissionPhase::OnMissionTimeUp()
{
	UE_LOG(LogTemp, Warning, TEXT("[VGMissionPhase] 미션 페이즈 시간 종료"));
	ExecutePhaseResult();
}
