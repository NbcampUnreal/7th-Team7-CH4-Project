#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionStatueAlign.generated.h"

UCLASS()
class VIGILANT_API AVGMissionStatueAlign : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionStatueAlign();

protected:
	// [Fix] Base 클래스에서 이미 UFUNCTION() 선언됨 — override에 중복 지정 제거
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;

private:
	bool AreAllStatuesAligned() const;
};
