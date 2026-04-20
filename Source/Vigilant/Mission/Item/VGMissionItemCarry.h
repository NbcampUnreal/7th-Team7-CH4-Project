#pragma once

#include "CoreMinimal.h"
#include "VGMissionItemBase.h"
#include "VGMissionItemCarry.generated.h"

class AVGMissionGimmickBase;

USTRUCT()
struct VIGILANT_API FVGCarryPlaceInfo
{
	GENERATED_BODY()
	
public:
	TObjectPtr<AActor> AttachmentTargetActor;
	FVector RelativeLocation;
};

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
	void OnRep_PlaceInfo();
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_PlaceInfo)
	FVGCarryPlaceInfo PlaceInfo;
};
