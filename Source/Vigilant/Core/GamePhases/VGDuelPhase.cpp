#include "VGDuelPhase.h"
#include "Core/VGGameMode.h"
#include "Character/VGCharacterBase.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"

void UVGDuelPhase::EnterPhase()
{
	Super::EnterPhase();
	
	if (!GameModeRef) return;
	
	AVGCharacterBase* Player1 = GameModeRef->GetDuelChallenger();
	AVGCharacterBase* Player2 = GameModeRef->GetDuelTarget();
	
	if (Player1 && Player2)
	{
		// 전체 플레이어 순회
		for (APlayerState* PlayerState : GameModeRef->GetWorld()->GetGameState()->PlayerArray)
		{
			AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
			if (!VGPlayerState) continue;

			AVGCharacterBase* Character = Cast<AVGCharacterBase>(VGPlayerState->GetPawn());
			if (!Character) continue;

			// 대상이 참가자인 경우
			if (Character == Player1 || Character == Player2)
			{
				// 참가자 태그 부여
				VGPlayerState->AddPlayerTag(VigilantStateTags::DuelParticipant);
				
				// 참가자를 막고라장으로 텔레포트하는 로직 추가 예정
				
			}
			// 대상이 참가자가 아니라면
			else
			{
				// 관객 태그 부여
				VGPlayerState->AddPlayerTag(VigilantStateTags::DuelSpectator);

				// 관객 텔레포르 로직 추가 예정
				
			}
			
		}

		UE_LOG(LogTemp, Warning, TEXT("[VGDuelPhase] 태그 부여 완료. 막고라 페이즈 시작"));
	
	}
}

void UVGDuelPhase::ExitPhase()
{
	// 플레이어들 막고라 관련 태그 해제
	for (APlayerState* PS : GameModeRef->GetWorld()->GetGameState()->PlayerArray)
	{
		if (AVGPlayerState* VGPS = Cast<AVGPlayerState>(PS))
		{
			VGPS->RemovePlayerTag(VigilantStateTags::DuelParticipant);
			VGPS->RemovePlayerTag(VigilantStateTags::DuelSpectator);
		}
	}
	
	if (GameModeRef)
	{
		GameModeRef->ClearDuelParticipants();
		
	}
	
	Super::ExitPhase();
}

void UVGDuelPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->TransitionToPhase(nullptr);
	}
}

bool UVGDuelPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	if (!InteractableObject)
	{
		return false;
	}
	if (InteractableObject->IsA(AVGCharacterBase::StaticClass()))
	{
		return false;
	}
	
	return true;
}

bool UVGDuelPhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	if (!Target) return false;

	AVGPlayerState* TargetPlayerState = Target->GetPlayerState<AVGPlayerState>();
	if (!TargetPlayerState) return false;

	// 관객은 데미지를 받지 않음
	if (TargetPlayerState->HasPlayerTag(VigilantStateTags::DuelSpectator))
	{
		return false;
	}

	// 참가자는 데미지를 받음
	return true;
}

void UVGDuelPhase::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	if (!GameModeRef) return;
	
	if (!Victim) return;

	AVGPlayerState* VictimPlayerState = Victim->GetPlayerState<AVGPlayerState>();
	if (!VictimPlayerState) return;

	// 죽은 사람이 막고라 참가자인지 확인
	if (VictimPlayerState->HasPlayerTag(VigilantStateTags::DuelParticipant))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGDuelPhase] 참가자 %s 사망! 막고라 종료"), *VictimPlayerState->GetPlayerName());

		// 후에 진 사람 채팅 불가 등 디버프 추가 예정
		
		ExecutePhaseResult();
	}
}
