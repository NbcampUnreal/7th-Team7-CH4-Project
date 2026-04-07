#pragma once

#include "CoreMinimal.h"
#include "VGMissionItemBase.h"
#include "VGMissionItemKey.generated.h"

UCLASS()
class VIGILANT_API AVGMissionItemKey : public AVGMissionItemBase
{
	GENERATED_BODY()

public:
	AVGMissionItemKey();
	
protected:
	virtual void OnRep_ItemStateTag() override;
};
