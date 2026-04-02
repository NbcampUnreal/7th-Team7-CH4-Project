// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickPressure.generated.h"

class UBoxComponent;

UCLASS()
class VIGILANT_API AVGMissionGimmickPressure : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickPressure();
	
	int32 GetSequenceIndex() const { return SequenceIndex; }
protected:
	virtual void BeginPlay() override;
	
	// IVGInteractable 구현 — 발판은 Overlap으로 감지하므로 직접 상호작용 없음
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;
	
	virtual void OnRep_GimmickStateTag() override;
	
	// 발판이 눌렸을 때 - 서버 전용
	void OnPressed();
	
	// 반판에서 벗어났을 때 - 서버 전용
	void OnReleased();
	
private:
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
protected:
	// Overlap 감지용 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Pressure")
	TObjectPtr<UBoxComponent> TriggerBox;
	
	// 발판을 밟을 수 있는 클래스 제한 - 에디터 필수 지정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission|Collision")
	TSubclassOf<AActor> TargetClass;
	
	// 서버 전용 - 클라이언트에서 직접 접근 금지
	// 발판을 밟고 있는 액터 목록 — 복수 인원 협동 발판 대응
	UPROPERTY()
	TArray<TObjectPtr<AActor>> OverlappingActors;
	
	// 발판 활성화에 필요한 최소 인원 수 (기본 1, 에디터에서 조정 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Pressure")
	int32 RequiredActorCount = 1;
	
	// 이 발판의 순서 번호 — 에디터에서 지정 (0부터 시작)
	// 순서 무관 미션에서는 사용 안 함
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gimmick|Pressure")
	int32 SequenceIndex = 0;
};
