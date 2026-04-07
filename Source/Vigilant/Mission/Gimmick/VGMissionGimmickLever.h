#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickLever.generated.h"

UCLASS()
class VIGILANT_API AVGMissionGimmickLever : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickLever();
	
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Lever")
	bool IsActivated() const;
	
	// 상호작용할 때마다 On/Off 토글
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;

protected:
	void Toggle();
	
	virtual void OnRep_GimmickStateTag() override;

public:
	// true이면 On 이후 Off 불가 (횃불 등에 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Lever")
	bool bIsOneWay = false;
	
};
