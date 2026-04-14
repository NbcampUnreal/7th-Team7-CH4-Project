#include "VGMissionGimmickChest.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
#include "Components/TimelineComponent.h"
#include "Net/UnrealNetwork.h"

AVGMissionGimmickChest::AVGMissionGimmickChest()
{
	PrimaryActorTick.bCanEverTick = false;
	GimmickTypeTag = VigilantMissionTags::ChestGimmick;
	
	// 뚜껑 메시 — 피봇을 경첩 위치에 맞춰야 자연스럽게 열림
	LidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMeshComponent"));
	LidMeshComponent->SetupAttachment(MeshComponent);
 
	LidTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("LidTimeline"));
}

void AVGMissionGimmickChest::BeginPlay()
{
	Super::BeginPlay();
	
	// 뚜껑 커브 미설정 시 0.5초 Linear로 자동 생성
	if (!LidOpenCurve)
	{
		LidOpenCurve = NewObject<UCurveFloat>(this);
		LidOpenCurve->FloatCurve.AddKey(0.f, 0.f);
		LidOpenCurve->FloatCurve.AddKey(0.5f, 1.f);
	}
	
	FOnTimelineFloat UpdateCallback;
	UpdateCallback.BindUFunction(this, FName("OnLidTimelineUpdate"));
	LidTimeline->AddInterpFloat(LidOpenCurve, UpdateCallback);
 
	FOnTimelineEvent FinishedCallback;
	FinishedCallback.BindUFunction(this, FName("OnLidTimelineFinished"));
	LidTimeline->SetTimelineFinishedFunc(FinishedCallback);
 
	// 뚜껑 초기 각도
	LidMeshComponent->SetRelativeRotation(FRotator(0.f, 0.f, LidClosedRoll));
 
	InitDynamicMaterials();
}

void AVGMissionGimmickChest::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bChestOpened);
}

bool AVGMissionGimmickChest::CanInteractWith(AActor* Interactor) const
{
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		return false;
	}
	
	UVGEquipmentComponent* EquipComp =
		Interactor ? Interactor->FindComponentByClass<UVGEquipmentComponent>() : nullptr;
	if (!EquipComp)
	{
		return false;
	}
	
	// GimmickBase 헬퍼: 양손 중 RequiredItemTypeTag와 일치하는 아이템 탐색
	return FindMissionItemByTag(EquipComp, RequiredItemTypeTag) != nullptr;
}

void AVGMissionGimmickChest::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority() || !CanInteractWith(Interactor))
	{
		return;
	}

	// 열쇠 아이템 찾아서 사용 처리
	UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}
	
	// 왼손 → 오른손 순으로 사용 가능한 슬롯 탐색
	const EVGEquipmentSlot Slots[] = { EVGEquipmentSlot::LeftHand, EVGEquipmentSlot::RightHand };
	AVGEquippableActor* HandItems[] = { EquipComp->LeftHandItem, EquipComp->RightHandItem };
 
	for (int32 i = 0; i < 2; ++i)
	{
		AVGMissionItemBase* MissionItem =
			Cast<AVGMissionItemBase>(HandItems[i]);
		if (!MissionItem)
		{
			continue;
		}
 
		UVGMissionItemDataAsset* ItemData =
			Cast<UVGMissionItemDataAsset>(MissionItem->EquipmentData);
		if (!ItemData || ItemData->ItemTypeTag != RequiredItemTypeTag)
		{
			continue;
		}
 
		MissionItem->SetStateTag(VigilantMissionTags::ItemUsed);
		EquipComp->Server_DropItem(Slots[i]);
		
		break;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[%s] Chest Open!"), *GetName());
	SetStateTag(VigilantMissionTags::GimmickCompleted);
	OnGimmickInteracted.Broadcast(this, Interactor);
}

void AVGMissionGimmickChest::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LidMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (bChestOpened == false)
		{
			LidTimeline->PlayFromStart();
		}
	}
}

// 타임라인 — 뚜껑 열림
void AVGMissionGimmickChest::OnLidTimelineUpdate(float Value)
{
	// Value 0→1 : LidClosedRoll → LidOpenRoll 보간
	float CurrentRoll = FMath::Lerp(LidClosedRoll, LidOpenRoll, Value);
	LidMeshComponent->SetRelativeRotation(FRotator(0.f, 0.f, CurrentRoll));
}

void AVGMissionGimmickChest::OnLidTimelineFinished()
{
	// 뚜껑이 완전히 열렸을 때
	// 서버: 보상 스폰 (MissionBase::SpawnRewardItems는 Mission에서 처리하므로
	//        여기서는 콜리전만 끄고 디졸브 시작)
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LidMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	bChestOpened = true;
	OnRep_bChestOpened();
}

void AVGMissionGimmickChest::OnRep_bChestOpened()
{
	if (bChestOpened)
	{
		StartDissolve();
	}
}

// 디졸브 — 타이머 기반
void AVGMissionGimmickChest::StartDissolve()
{
	DissolveElapsed = 0.f;
 
	GetWorldTimerManager().SetTimer(
		DissolveTimerHandle,
		this,
		&AVGMissionGimmickChest::OnDissolveTimerTick,
		DissolveTickInterval,
		true // 반복
	);
}

void AVGMissionGimmickChest::OnDissolveTimerTick()
{
	DissolveElapsed += DissolveTickInterval;
	float Alpha = FMath::Clamp(DissolveElapsed / DissolveDuration, 0.f, 1.f);
 
	if (BodyDynMat)
	{
		BodyDynMat->SetScalarParameterValue(DissolveParamName, Alpha);
	}
	if (LidDynMat)
	{
		LidDynMat->SetScalarParameterValue(DissolveParamName, Alpha);
	}
 
	// 디졸브 완료
	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
 
		// 서버에서만 액터 제거 — 클라이언트는 복제 소멸로 자동 처리
		if (HasAuthority())
		{
			SetLifeSpan(0.01f);
		}
	}
}

void AVGMissionGimmickChest::InitDynamicMaterials()
{
	// 본체와 뚜껑 각각 슬롯 0번에서 동적 머티리얼 생성
	// 본체는 부모 클래스에서 생성
	// 머티리얼이 없으면 nullptr — 디졸브 단계에서 null 체크로 안전하게 처리
 
	if (UMaterialInterface* LidMat = LidMeshComponent->GetMaterial(0))
	{
		LidDynMat = UMaterialInstanceDynamic::Create(LidMat, this);
		LidMeshComponent->SetMaterial(0, LidDynMat);
	}
}
