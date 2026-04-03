// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANSVGIframe.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API UANSVGIframe : public UAnimNotifyState
{
	GENERATED_BODY()
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("I-Frame");
	}
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IFrame")
	FGameplayTag IFrameTag;
};
