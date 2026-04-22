#include "VGDuelPhase.h"
#include "Core/VGGameMode.h"
#include "Character/VGCharacterBase.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Component/VGStatComponent.h"

void UVGDuelPhase::EnterPhase()
{
	Super::EnterPhase();
	
	bIsDuelFinished = false;
	
	if (!GameModeRef) return;
	
	AVGCharacterBase* Player1 = GameModeRef->GetDuelChallenger();
	AVGCharacterBase* Player2 = GameModeRef->GetDuelTarget();
	
	if (!Player1 || !Player2) return;
	
	// 아레나 스폰 포인트 검색 및 분류
	TArray<AActor*> AllStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AllStarts);
	
	ParticipantStarts.Empty();
	SpectatorStarts.Empty();
	
	for (AActor* Start : AllStarts)
	{
		if (APlayerStart* PlayerStart = Cast<APlayerStart>(Start))
		{
			if (PlayerStart->PlayerStartTag == FName("DuelParticipant"))
			{
				ParticipantStarts.Add(PlayerStart);
			}
			else if (PlayerStart->PlayerStartTag == FName("DuelSpectator"))
			{
				SpectatorStarts.Add(PlayerStart);
			}
		}
	}
	
	int32 ParticipantIndex = 0;
	int32 SpectatorIndex = 0;
	
	// 전체 플레이어 순회(태그 부여, 텔레포트 진행)
	for (APlayerState* PlayerState : GameModeRef->GetWorld()->GetGameState()->PlayerArray)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
		if (!VGPlayerState) continue;

		AVGCharacterBase* VGCharacter = Cast<AVGCharacterBase>(VGPlayerState->GetPawn());
		if (!VGCharacter) continue;

		// 막고라장으로 옮기기전 플레이어들 Transform 정보 저장
		OriginalTransforms.Add(VGPlayerState, VGCharacter->GetActorTransform());

		AActor* TargetStart = nullptr;

		// 참가자일 때 태그 부여 및 스폰포인트 지정
		if (VGCharacter == Player1 || VGCharacter == Player2)
		{
			VGPlayerState->AddPlayerTag(VigilantStateTags::DuelParticipant);
			TargetStart = ParticipantStarts.IsValidIndex(ParticipantIndex) ? ParticipantStarts[ParticipantIndex++] : nullptr;
		}
		// 참가자 아닐 때 태그 부여 및 스폰포인트 지정
		else
		{
			VGPlayerState->AddPlayerTag(VigilantStateTags::DuelSpectator);
			TargetStart = SpectatorStarts.IsValidIndex(SpectatorIndex) ? SpectatorStarts[SpectatorIndex++] : nullptr;
		}

		// 배정된 PlayerStart로 이동
		if (TargetStart)
		{
			FVector Loc = TargetStart->GetActorLocation();
			FRotator Rot = TargetStart->GetActorRotation();

			if (APlayerController* PlayerController = Cast<APlayerController>(VGCharacter->GetController()))
			{
				PlayerController->SetControlRotation(Rot);
				PlayerController->ClientSetRotation(Rot);
			}
			VGCharacter->TeleportTo(Loc, Rot, false, true);
			VGCharacter->Client_ForceRotation(Rot, false);
		}
	}
	
	if (UVGStatComponent* Stat1 = Player1->GetStatComponent())
	{
		Stat1->OnHPChanged.AddUniqueDynamic(this, &UVGDuelPhase::OnChallengerHPChanged);
	}
	if (UVGStatComponent* Stat2 = Player2->GetStatComponent())
	{
		Stat2->OnHPChanged.AddUniqueDynamic(this, &UVGDuelPhase::OnTargetHPChanged);
	}
	

}

void UVGDuelPhase::ExitPhase()
{
	AVGCharacterBase* Player1 = GameModeRef->GetDuelChallenger();
	AVGCharacterBase* Player2 = GameModeRef->GetDuelTarget();
	
	if (Player1 && Player1->GetStatComponent())
	{
		Player1->GetStatComponent()->OnHPChanged.RemoveDynamic(this, &UVGDuelPhase::OnChallengerHPChanged);
	}
	if (Player2 && Player2->GetStatComponent())
	{
		Player2->GetStatComponent()->OnHPChanged.RemoveDynamic(this, &UVGDuelPhase::OnTargetHPChanged);
	}
	
	// 플레이어 원위치 복귀
	for (auto& Pair : OriginalTransforms)
	{
		AVGPlayerState* VGPlayerState = Pair.Key;
		FTransform OriginalTarget = Pair.Value;

		if (VGPlayerState && VGPlayerState->GetPawn())
		{
			APawn* Pawn = VGPlayerState->GetPawn();
			FVector Loc = OriginalTarget.GetLocation();
			FRotator Rot = OriginalTarget.GetRotation().Rotator();

			// 시점 동기화
			if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				PlayerController->SetControlRotation(Rot);
				PlayerController->ClientSetRotation(Rot);
			}
			Pawn->TeleportTo(Loc, Rot, false, true);
		}
	}
	
	// 메모리 정리
	OriginalTransforms.Empty();
	ParticipantStarts.Empty();
	SpectatorStarts.Empty();
	
	// 플레이어들 막고라 관련 태그 해제
	for (APlayerState* PlayerState : GameModeRef->GetWorld()->GetGameState()->PlayerArray)
	{
		if (AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState))
		{
			VGPlayerState->RemovePlayerTag(VigilantStateTags::DuelParticipant);
			VGPlayerState->RemovePlayerTag(VigilantStateTags::DuelSpectator);
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
		GameModeRef->NotifyPhaseCompleted(this);
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

bool UVGDuelPhase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
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
}

void UVGDuelPhase::OnChallengerHPChanged(float NewHP, float MaxHP)
{
	if (MaxHP > 0.f && (NewHP / MaxHP) < 0.2f)
	{
		if (GameModeRef)
		{
			HandleDuelDefeat(GameModeRef->GetDuelChallenger());
		}
	}
		
}

void UVGDuelPhase::OnTargetHPChanged(float NewHP, float MaxHP)
{
	if (MaxHP > 0.f && (NewHP / MaxHP) < 0.2f)
	{
		if (GameModeRef)
		{
			HandleDuelDefeat(GameModeRef->GetDuelTarget());
		}
	}
}

void UVGDuelPhase::HandleDuelDefeat(AVGCharacterBase* DefeatedPlayer)
{
	if (bIsDuelFinished) return;
	bIsDuelFinished = true;

	// 참가자 중 한 명 체력 20% 밑으로 줄어들면 바로 종료
	ExecutePhaseResult();
}
