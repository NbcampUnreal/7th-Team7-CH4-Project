// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGInteractionWidget.generated.h"

class UOverlay;
class UTextBlock;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InteractText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> InteractOverlay;
public:
	void SetInteractText(const FString& InfoText);
	void HideInteract();
	
	void SetTargetWorldLocation(const FVector& InLocation);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	// 타겟의 3D 월드 위치를 기억할 변수
	FVector TargetWorldLocation = FVector::ZeroVector;
};
