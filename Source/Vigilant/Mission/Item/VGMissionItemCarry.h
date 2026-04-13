#pragma once

#include "CoreMinimal.h"
#include "VGMissionItemBase.h"
#include "VGMissionItemCarry.generated.h"

class AVGMissionGimmickBase;

UCLASS()
class VIGILANT_API AVGMissionItemCarry : public AVGMissionItemBase
{
	GENERATED_BODY()

public:
	AVGMissionItemCarry();
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 목표 위치에 놓였을 때 호출 — Gimmick에서 호출
	void PlaceOnTarget(AVGMissionGimmickBase* TargetGimmick, FVector TargetRelativeLocation);
	
	UFUNCTION()
	void OnRep_AttachmentTargetActor();
	UFUNCTION()
	void OnRep_AttachmentRelativeLocation();
	
protected:
	virtual void OnRep_Carrier() override;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_AttachmentTargetActor)
	TObjectPtr<AActor> AttachmentTargetActor;
	
	UPROPERTY(ReplicatedUsing=OnRep_AttachmentRelativeLocation)
	FVector AttachmentRelativeLocation;
	
};
