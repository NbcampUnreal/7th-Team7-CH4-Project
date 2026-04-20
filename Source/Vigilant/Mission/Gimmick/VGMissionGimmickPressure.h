// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickPressure.generated.h"

class UBoxComponent;
class UTimelineComponent;

UCLASS()
class VIGILANT_API AVGMissionGimmickPressure : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickPressure();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnRep_GimmickStateTag() override;
	
	// 발판이 눌렸을 때 - 서버 전용
	void OnPressed();
	
	// 반판에서 벗어났을 때 - 서버 전용
	void OnReleased();
	
	void PlayPressAnimation();
	
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
	
	// 타임라인 콜백
	UFUNCTION()
	void OnPressTimelineUpdate(float Value);
	
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
	
	// 미션 종류에 따라 동작 방식 선택 가능하도록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Pressure")
	bool bToggleMode = false; // true면 재밟기 시 비활성화 (마피아 방해용)
	
	// true면 발판에서 벗어날 때 Inactive로 복귀, false면 한번 눌리면 상태 유지 (일회성 트리거)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Pressure")
	bool bRevertOnRelease = false;
	
	// 눌렸을 때 내려가는 깊이 (cm)
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Pressure|Animation")
	float PressDepth = 8.f;
	// 보간 속도용 커브 (0→1)
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Pressure|Animation")
	TObjectPtr<UCurveFloat> PressCurve;
	
private:
	UPROPERTY()
	TObjectPtr<UTimelineComponent> PressTimeline;
 
	FVector OriginalRelativeLocation;
};
