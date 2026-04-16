#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickLever.generated.h"

class UTimelineComponent;

UCLASS()
class VIGILANT_API AVGMissionGimmickLever : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickLever();
	
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Lever")
	bool IsActivated() const;
	
	// 상호작용할 때마다 On/Off 토글
	virtual bool CanInteractWith(AActor* Interactor) const override;
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform) override;

protected:
	virtual void BeginPlay() override;
	
	void Toggle();
	
	// 레버 회전 타임라인 콜백
	UFUNCTION()
	void OnLeverTimelineUpdate(float Value);
 
	UFUNCTION()
	void OnLeverTimelineFinished();
	
	virtual void OnRep_GimmickStateTag() override;
	
	void PlayLeverAnimation();
	
public:
	// true이면 On 이후 Off 불가 (횃불 등에 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Lever")
	bool bIsOneWay = false;
	
protected:
	// 손잡이 매쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Chest",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LeverMeshComponent;
	
	UPROPERTY()
	TObjectPtr<UTimelineComponent> LeverTimeline;
	
	// 에디터에서 Float Curve 연결 (0→1 구간)
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Lever")
	TObjectPtr<UCurveFloat> LeverRotationCurve;
 
	// 레버가 내려간(비활성) 상태의 Pitch 각도
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Lever")
	float InactivePitch = 200.f;
 
	// 레버가 올라간(활성) 상태의 Pitch 각도
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Lever")
	float ActivePitch = 359.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Gimmick|Lever")
	float TargetPitch = 0.f;
	
	// 손잡이 머티리얼
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HandleDynMat;
};
