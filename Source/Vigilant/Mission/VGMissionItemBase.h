#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/VGInteractable.h"
#include "VGMissionItemBase.generated.h"

class AVGMissionBase;
class AVGCharacterBase;

UCLASS(Abstract)
class VIGILANT_API AVGMissionItemBase : public AActor, public IVGInteractable
{
	GENERATED_BODY()

public:
	AVGMissionItemBase();

	// IVGInteractable 구현

	UFUNCTION(BlueprintCallable)
	AVGCharacterBase* GetCarrier() const { return Carrier; }

	UFUNCTION(BlueprintCallable)
	bool IsCarried() const { return Carrier != nullptr; }

protected:
	// 조건 충족 시 보고할 대상 미션 — 에디터에서 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<AVGMissionBase> OwnerMission;

	// 현재 이 아이템을 들고 있는 캐릭터
	// Replicated: 아이템이 캐릭터를 따라 움직이는 시각 처리를 모든 클라이언트에 동기화
	UPROPERTY(ReplicatedUsing = OnRep_Carrier)
	TObjectPtr<AVGCharacterBase> Carrier;

	// 줍기 — 서버 전용
	virtual void OnPickedUp(AVGCharacterBase* NewCarrier);

	// 내려놓기 — 서버 전용
	virtual void OnDropped();

	// 목표 위치 도달 등 조건 충족 시 자식 클래스에서 호출
	void ReportConditionMet();

	UFUNCTION()
	virtual void OnRep_Carrier();

public:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};