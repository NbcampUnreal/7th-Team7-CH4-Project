#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/VGGameplayTags.h"
#include "VGMissionDataAsset.generated.h"

class AVGEquippableActor;

UCLASS()
class VIGILANT_API UVGMissionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	int32 MissionID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	FString MissionDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	FGameplayTag MissionTypeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Reward")
	TSubclassOf<AVGEquippableActor> RewardItemClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|TimeAttack")
	float ClearReduceTime = 30.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|TimeAttack")
	float TimeLimit = 30.f;
};
