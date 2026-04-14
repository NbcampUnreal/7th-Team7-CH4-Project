#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "VGMissionGimmickChest.generated.h"

class UTimelineComponent;
class AVGEquippableActor;

UCLASS()
class VIGILANT_API AVGMissionGimmickChest : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickChest();

	virtual bool CanInteractWith(AActor* Interactor) const override;
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform) override;

protected:
	virtual void BeginPlay() override;
	
	void GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
	virtual void OnRep_GimmickStateTag() override;
	
	// ---- 타임라인 콜백 ----
	UFUNCTION()
	void OnLidTimelineUpdate(float Value);
 
	UFUNCTION()
	void OnLidTimelineFinished();
	
	UFUNCTION()
	void OnRep_bChestOpened();
	
private:
	// 디졸브를 타이머로 진행 (Tick 대신 타이머 — 항상 켜진 Tick 회피)
	void StartDissolve();
 
	UFUNCTION()
	void OnDissolveTimerTick();
 
	// 동적 머티리얼 초기화 (BeginPlay에서 한 번만)
	void InitDynamicMaterials();
	
protected:
	// 상자가 완전히 열렸는 지 체크용? 상자가 열리기 시작하는 것은 GimmickStateTag == GimmickCompleted일 때이니
	UPROPERTY(ReplicatedUsing = OnRep_bChestOpened)
	bool bChestOpened = false;
	
	// 뚜껑 매쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Chest",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LidMeshComponent;
 
	UPROPERTY()
	TObjectPtr<UTimelineComponent> LidTimeline;
	
	// ---- 에디터 설정 ----
 
	// 뚜껑 열림 보간 커브 (0→1, 미설정 시 Linear 자동 생성)
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	TObjectPtr<UCurveFloat> LidOpenCurve;
 
	// 뚜껑이 닫힌 상태의 Roll
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	float LidClosedRoll = 0.f;
 
	// 뚜껑이 완전히 열린 상태의 Roll — 경첩 축 방향에 따라 음수일 수 있음
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	float LidOpenRoll = 80.f;
 
	// 디졸브 총 소요 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	float DissolveDuration = 1.5f;
 
	// 머티리얼의 디졸브 스칼라 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	FName DissolveParamName = TEXT("DissolveProgress");
	
	// 뚜껑 머티리얼
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LidDynMat;
	
	FTimerHandle DissolveTimerHandle;
	float DissolveElapsed = 0.f;
	
	static constexpr float DissolveTickInterval = 0.033f;
	
	// 열쇠 아이템 타입 태그 — 이 태그를 가진 아이템만 사용 가능
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	FGameplayTag RequiredItemTypeTag;
};
