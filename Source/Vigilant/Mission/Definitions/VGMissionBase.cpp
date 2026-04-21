#include "VGMissionBase.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Mission/VGMissionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Data/VGMissionDataAsset.h"

AVGMissionBase::AVGMissionBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	
	bSpawnRewardAtMission = false;
	
	// 초기 상태는 비활성
	CurrentStateTag = VigilantMissionTags::MissionInactive;
}

void AVGMissionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentStateTag);
}

void AVGMissionBase::RegisterContributor(AVGCharacterBase* Character)
{
	if (!HasAuthority() || !Character) return;
	Contributors.AddUnique(Character);
	LastContributor = Character;
}

void AVGMissionBase::UnregisterContributor(AVGCharacterBase* Character)
{
	if (!HasAuthority() || !Character)
	{
		return;
	}
	
	Contributors.Remove(Character); // TWeakObjectPtr 배열에서 제거
    
	// LastContributor가 제거 대상이면 갱신
	if (LastContributor == Character)
	{
		LastContributor = Contributors.IsEmpty() ?
			nullptr : Contributors.Last().Get();
	}
}

void AVGMissionBase::ClearContributers()
{
	Contributors.Empty();
	LastContributor = nullptr;
	UE_LOG(LogTemp, Log, TEXT("[%s] Mission Failed. Clear Contributors."), *GetName());
}

void AVGMissionBase::OnGimmickInteracted(AVGMissionGimmickBase* Gimmick, AActor* Interactor)
{
	if (Interactor && Gimmick)
	{
		AVGCharacterBase* Character = Cast<AVGCharacterBase>(Interactor);
		if (!Character)
		{
			return;
		}
        
		if (Gimmick->GetStateTag() == VigilantMissionTags::GimmickActive
			|| Gimmick->GetStateTag() == VigilantMissionTags::GimmickCompleted)
		{
			RegisterContributor(Character);
		}
		else
		{
			UnregisterContributor(Character);
		}
	}
}

void AVGMissionBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (UVGMissionSubsystem* Subsystem =
			GetWorld()->GetSubsystem<UVGMissionSubsystem>())
		{
			Subsystem->Server_RegisterMission(this);
		}
		
		SetMissionState(VigilantMissionTags::MissionInactive);
		
		// 미션 인덱스 교체
		ShuffleGimmickIndexes();
		
		// 에디터에서 등록된 기믹들에 바인딩
		for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
		{
			if (Gimmick)
			{
				Gimmick->OnGimmickStateChanged.AddDynamic(
					this, &AVGMissionBase::OnGimmickStateChanged);
				Gimmick->OnGimmickInteracted.AddDynamic(
					this, &AVGMissionBase::OnGimmickInteracted);
			}
		}
		
		// 에디터에서 등록된 Item 바인딩
		for (AVGMissionItemBase* Item : MissionItems)
		{
			if (Item)
			{
				Item->OnItemStateChanged.AddDynamic(
					this, &AVGMissionBase::OnItemStateChanged);
			}
		}
	}
	else
	{
		// 클라이언트: OnRep_MissionID 대신 BeginPlay에서 직접 등록
		if (UVGMissionSubsystem* Subsystem =
			GetWorld()->GetSubsystem<UVGMissionSubsystem>())
		{
			Subsystem->Client_RegisterMission(this);
		}
	}
}

void AVGMissionBase::SetMissionState(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentStateTag = NewStateTag;

	// 서버는 OnRep가 자동 호출되지 않으므로 직접 호출
	OnRep_CurrentStateTag();
	
	// 완료 상태를 외부에 전달
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		NotifyMissionCompleted();
	}
}

bool AVGMissionBase::HasMissionTag(FGameplayTag Tag) const
{
	if (GetMissionTypeTag() == Tag)
	{
		return true;
	}
	else if (GetCurrentStateTag() == Tag)
	{
		return true;
	}
	
	// MissionTags 컨테이너 포함 여부 확인
	return MissionTags.HasTag(Tag);
}

int32 AVGMissionBase::GetMissionID() const
{
	return MissionData ? MissionData->MissionID : -1; 
}

FGameplayTag AVGMissionBase::GetMissionTypeTag() const
{
	return MissionData ? MissionData->MissionTypeTag : VigilantMissionTags::PuzzleMission;
}

void AVGMissionBase::OnRep_CurrentStateTag()
{
	OnMissionStateChanged.Broadcast(GetMissionID(), CurrentStateTag);
}

void AVGMissionBase::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
}

void AVGMissionBase::OnItemStateChanged(AVGMissionItemBase* Item, FGameplayTag Tag)
{
}

void AVGMissionBase::ShuffleGimmickIndexes()
{
	TArray<int32> Indexes;
	for (int32 i = 0; i < MissionGimmicks.Num(); i++)
	{
		Indexes.Add(i);
	}
		
	// 에디터에서 등록된 기믹들에 바인딩
	for (int32 i = 0; i < MissionGimmicks.Num(); i++)
	{
		int32 Index = FMath::RandRange(0,Indexes.Num()-1);
		AVGMissionGimmickBase* Gimmick = MissionGimmicks[i];
		if (Gimmick)
		{
			Gimmick->SetGimmickIndex(Indexes[Index]); // 자동 인덱스 부여
		}
		
		// RemoveAt 보다 효율적인 함수로 변경
		Indexes.RemoveAtSwap(Index);
	}
}

void AVGMissionBase::SpawnRewardItems()
{
	// 스폰은 서버에서만 진행
	if (!HasAuthority())
	{
		return;
	}
	
	// 기본 구현: LastContributor 주변에 아이템 스폰
	// 자식 클래스에서 override하여 커스텀
	if (GetRewardItemClass() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RewardItemClass is missing."), *GetName());
		return;
	}
	
	FVector SpawnLocation = GetActorLocation();
	if (!bSpawnRewardAtMission)
	{
		if (!LastContributor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] LastContributor is missing; falling back to mission location."), *GetName());
		}
		else
		{
			SpawnLocation = LastContributor->GetActorLocation()
							  + LastContributor->GetActorForwardVector() * 100.f;
			SpawnLocation.Z += 50.f;
		}
	}
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AVGEquippableActor>(GetRewardItemClass(), SpawnLocation,
								   FRotator::ZeroRotator, Params);
}

void AVGMissionBase::CompleteMission()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		return;
	}
    
	SetMissionState(VigilantMissionTags::MissionCompleted);
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (Gimmick)
		{
			Gimmick->SetStateTag(VigilantMissionTags::GimmickCompleted);
		}
	}
	
	GetWorldTimerManager().ClearTimer(MissionTimerHandle);
	
	// 보상 지급
	SpawnRewardItems();
}

void AVGMissionBase::NotifyMissionCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Mission Completed!"), *GetName());
	// UI 및 외부 시스템용 델리게이트 브로드캐스트
	OnMissionCompleted.Broadcast(GetMissionID());
}

FString AVGMissionBase::GetMissionDescription() const
{
	return MissionData ? MissionData->MissionDescription : TEXT(""); 
}

float AVGMissionBase::GetMissionTimeLimit() const
{
	return MissionData ? MissionData->TimeLimit : 0.f; 
}

TSubclassOf<AVGEquippableActor> AVGMissionBase::GetRewardItemClass() const
{
	return MissionData ? MissionData->RewardItemClass : nullptr; 
}

float AVGMissionBase::GetMissionClearReduceTime() const
{
	return MissionData ? MissionData->ClearReduceTime : 0.f; 
}
