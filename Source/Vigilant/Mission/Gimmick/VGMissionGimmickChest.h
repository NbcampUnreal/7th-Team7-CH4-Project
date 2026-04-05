#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickChest.generated.h"

UCLASS()
class VIGILANT_API AVGMissionGimmickChest : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickChest();

	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;

protected:
	// 열쇠 아이템 타입 태그 — 이 태그를 가진 아이템만 사용 가능
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	FGameplayTag RequiredItemTypeTag;
};
