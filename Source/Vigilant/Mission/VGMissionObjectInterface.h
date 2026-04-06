#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "VGMissionObjectInterface.generated.h"

class AVGMissionBase;

UINTERFACE()
class UVGMissionObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class VIGILANT_API IVGMissionObjectInterface
{
	GENERATED_BODY()

public:
	// Mission에서 BeginPlay 시 호출하여 OwnerMission 주입
	virtual void SetOwnerMission(AVGMissionBase* InOwnerMission) = 0;

	// 조건 충족 시 자식에서 호출 → OwnerMission에 보고
	virtual void ReportConditionMet() = 0;
	
	// 상태 태그 반환
	virtual FGameplayTag GetStateTag() = 0;
	virtual void SetStateTag(FGameplayTag NewStateTag) = 0;
};
