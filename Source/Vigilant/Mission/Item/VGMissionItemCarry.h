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
	
	// 목표 위치에 놓였을 때 호출 — Gimmick에서 호출
	void PlaceOnTarget(AVGMissionGimmickBase* TargetGimmick);
};
