#include "VGFinalCombatPhase.h"
#include "Character/VGCharacterBase.h"
#include "Common/VGGameplayTags.h"
#include "Core/VGGameMode.h"
#include "Core/VGPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void UVGFinalCombatPhase::EnterPhase()
{
	bHasTimeLimit = false;
	
	Super::EnterPhase();
	UE_LOG(LogTemp, Warning, TEXT("[VGFinalCombatPhase] 최후의 전투 시작"));
	
	if (!GameModeRef || !GameModeRef->GameState || !BossCharacterClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

	TArray<AActor*> AllStarts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), AllStarts);
	
    // 투기장 구역의 스폰 포인트 미리 찾기
    // 태그(Actor Tag)를 각각 BossSpawn, CitizenSpawn로 지정
    TArray<AActor*> BossStarts;
    TArray<AActor*> CitizenStarts;
	for (AActor* Start : AllStarts)
	{
		if (APlayerStart* PlayerStart = Cast<APlayerStart>(Start))
		{
			if (PlayerStart->PlayerStartTag == FName("BossSpawn"))
			{
				BossStarts.Add(PlayerStart);
			}
			else if (PlayerStart->PlayerStartTag == FName("CitizenSpawn"))
			{
				CitizenStarts.Add(PlayerStart);
			}
		}
	}

    // 보스 스폰 위치 (태그가 없으면 임시 좌표)
    FVector BossSpawnLoc = BossStarts.Num() > 0 ? BossStarts[0]->GetActorLocation() : FVector(0.0f, 0.0f, 100.0f);
    FRotator BossSpawnRot = BossStarts.Num() > 0 ? BossStarts[0]->GetActorRotation() : FRotator::ZeroRotator;

    int32 CitizenSpawnIndex = 0; // 시민들이 안 겹치게 하기 위한 인덱스

    // 접속 중인 플레이어 처리
    for (APlayerState* CurrentPlayerState : GameModeRef->GameState->PlayerArray)
    {
        AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(CurrentPlayerState);
        if (!VGPlayerState) continue;

        APlayerController* PlayerController = VGPlayerState->GetPlayerController();
        if (!PlayerController) continue;
    	

        APawn* CurrentPawn = PlayerController->GetPawn();

        // 마피아 유저 처리 (보스로 변신)
        if (VGPlayerState->IsRole(VigilantRoleTags::Mafia))
        {
            if (CurrentPawn) CurrentPawn->Destroy(); 

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            
            AVGCharacterBase* BossPawn = World->SpawnActor<AVGCharacterBase>(BossCharacterClass, BossSpawnLoc, BossSpawnRot, SpawnParams);
            
            if (BossPawn)
            {
                PlayerController->Possess(BossPawn);
                UE_LOG(LogTemp, Warning, TEXT("[VGFinalCombatPhase] 마피아 플레이어, 투기장 보스로 빙의"));
            }
        }
        // 시민 유저 처리 (투기장으로 순간이동)
        else
        {
            if (CurrentPawn)
            {
                // 시민 스폰 위치 계산
                FVector CitizenLoc = FVector(500.0f, CitizenSpawnIndex * 200.0f, 100.0f); // 기본 임시 좌표
                FRotator CitizenRot = FRotator::ZeroRotator;

                if (CitizenStarts.Num() > 0)
                {
                    // 시민 스폰 포인트가 여러 개면 순서대로 배정
                    int32 SafeIndex = CitizenSpawnIndex % CitizenStarts.Num();
                    CitizenLoc = CitizenStarts[SafeIndex]->GetActorLocation();
                	CitizenRot = FRotator(0.0f, CitizenStarts[SafeIndex]->GetActorRotation().Yaw, 0.0f);
                }
               
            	CurrentPawn->TeleportTo(CitizenLoc, CitizenRot, false, true);
            	
            	if (AVGCharacterBase* VGCharacter = Cast<AVGCharacterBase>(CurrentPawn))
            	{
            		VGCharacter->Client_ForceRotation(CitizenRot, false);
            	}
            	
                CitizenSpawnIndex++;
                
                UE_LOG(LogTemp, Log, TEXT("[VGFinalCombatPhase] 시민 플레이어, 투기장으로 이동"));
            }
        }
    }
}

void UVGFinalCombatPhase::ExitPhase()
{
	Super::ExitPhase();
}

void UVGFinalCombatPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->NotifyPhaseCompleted(this);
	}
}

bool UVGFinalCombatPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	return false;
}

bool UVGFinalCombatPhase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
	return true;
}

bool UVGFinalCombatPhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	AVGCharacterBase* Attacker = Cast<AVGCharacterBase>(DamageCauser);
	
	if (Attacker && Target)
	{
		AVGPlayerState* AttackerPlayerState = Attacker->GetPlayerState<AVGPlayerState>();
		AVGPlayerState* TargetPlayerState = Target->GetPlayerState<AVGPlayerState>();
		
		if (AttackerPlayerState && TargetPlayerState)
		{
			bool bAttackerIsCitizen = AttackerPlayerState->IsRole(VigilantRoleTags::Citizen);
			bool bTargetIsCitizen = TargetPlayerState->IsRole(VigilantRoleTags::Citizen);
			
			if (bAttackerIsCitizen && bTargetIsCitizen)
			{
				return false;
			}
		}
	}
	
	return true;
}

void UVGFinalCombatPhase::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	Super::OnPlayerDeath(Killer, Victim);
	
	if (Victim)
	{
		if (AVGPlayerState* VictimPlayerState = Victim->GetPlayerState<AVGPlayerState>())
		{
			VictimPlayerState->AddPlayerTag(VigilantRoleTags::Dead);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[VGFinalCombatPhase] 사망자 발생. 승리 조건 체크"));
	if (GameModeRef)
	{
		GameModeRef->CheckWinCondition();
	}
}