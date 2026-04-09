// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGVoteSlotWidget.generated.h"

//버튼이 눌렸을때 몇번 슬롯(플레이어)인지 전달하는 델리게이
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteSlotClicked, int32, ClickedIndex);
class UButton;
class UImage;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGVoteSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 부모가 이 슬롯을 생성할 때 번호를 부여해 줄 함수
	void SetupSlot(int32 InPlayerIndex, USceneCaptureComponent2D* TargetCamera);
	
	UPROPERTY()
	FOnVoteSlotClicked OnVoteSlotClickedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PortraitImage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> VoteButton;
	
	//몇번 째로 들어온지 확인하는 인덱스
	int32 PlayerIndex;
	// 버튼 눌렸을 때 실행될 함수
	UFUNCTION()
	void OnButtonClicked();
	
protected:
	// 콘텐츠 브라우저에 있는 '원본 머티리얼'을 담을 변수
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> BaseMaterial;
	
};
