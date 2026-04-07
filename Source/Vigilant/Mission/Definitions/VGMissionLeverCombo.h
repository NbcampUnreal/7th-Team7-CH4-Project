#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionLeverCombo.generated.h"

UCLASS()
class VIGILANT_API AVGMissionLeverCombo : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionLeverCombo();

protected:
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;

public:
	// 에디터에서 켜져 있어야 할 레버 목록 지정
	// MissionGimmicks 전체 중 이 목록에 있는 것만 Active여야 함
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Lever")
	TSet<int32> RequiredOnLeverIndexes;
	
private:
	// 현재 활성화된 레버 인덱스 목록 — 런타임 관리
	TSet<int32> CurrentOnLeverIndexes;
};
