#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VGMissionSandbag.generated.h"

class AVGCharacterBase;
class UVGStatComponent;
// 막타 플레이어 정보를 미션에 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FOnSandbagDefeated, AVGCharacterBase*, LastAttacker);

UCLASS()
class VIGILANT_API AVGMissionSandbag : public AActor
{
	GENERATED_BODY()

public:
	AVGMissionSandbag();
	// 데미지를 입힌 플레이어를 추적하기 위해 외부에서 직접 호출
	UFUNCTION(BlueprintCallable)
	void RegisterAttacker(AVGCharacterBase* Attacker);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnDead(AController* LastInstigator);

	UFUNCTION()
	void OnHPChanged(float NewHP, float MaxHP);
	
	UFUNCTION()
	void OnRep_CurrentHPRatio();
public:
	UPROPERTY(BlueprintAssignable)
	FOnSandbagDefeated OnSandbagDefeated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sandbag")
	TObjectPtr<UVGStatComponent> StatComponent;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 마지막으로 데미지를 준 플레이어 — 서버 전용
	UPROPERTY()
	TObjectPtr<AVGCharacterBase> LastAttacker;

	// HP 바 표시용 — Replicated
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHPRatio)
	float CurrentHPRatio = 1.f;
};
