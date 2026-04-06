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
	// 상태 태그 반환
	// [Fix] GetStateTag에 const 추가 — 상태 조회는 객체를 변경하지 않음
	virtual FGameplayTag GetStateTag() const = 0;
	virtual void SetStateTag(FGameplayTag NewStateTag) = 0;
};
