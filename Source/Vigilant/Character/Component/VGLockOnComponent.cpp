// Fill out your copyright notice in the Description page of Project Settings.


#include "VGLockOnComponent.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "GameplayTagAssetInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Common/VGGameplayTags.h"
// Sets default values for this component's properties
UVGLockOnComponent::UVGLockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVGLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


AActor* UVGLockOnComponent::FindBestTarget()
{
	IVGCharacterGameplayTagEditor* TagEditor = Cast<IVGCharacterGameplayTagEditor>(GetOwner());
	
	//오너 캐싱
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}
	// 카메라 캐싱
	UCameraComponent* FollowCamera = Owner->FindComponentByClass<UCameraComponent>();
	if (!FollowCamera)
	{
		return nullptr;
	}
	
	//SphereOverLapActors 사용 락온 대상 탐색
	//4번 매개변수 : 찾을 채널
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	//6번 매개변수 : 무시할 액터 배열
	TArray<AActor*> IgnoreTargets;
	IgnoreTargets.Add(Owner);

	bool bResult =
		UKismetSystemLibrary::SphereOverlapActors(
			this,
			Owner->GetActorLocation(),
			MaxLockOnDistance,
			ObjectTypes,
			APawn::StaticClass(),
			IgnoreTargets,
			LockOnTargetList);

	if (bResult)
	{
		AActor* BestTarget = nullptr;
		float HighestWeight = -1.0f;

		for (AActor* Target : LockOnTargetList)
		{
			FVector CameraLocation = FollowCamera->GetComponentLocation();
			FVector CameraDirection = FollowCamera->GetForwardVector();
			FVector TargetLocation = Target->GetActorLocation();
			//벡터 AB는 B-A
			FVector ToTargetVector = (TargetLocation - CameraLocation).GetSafeNormal();
			float TargetDistance = FVector::Dist(CameraLocation, TargetLocation);
			//카메라 시선과 타겟 방향 내적 - 1에 가까울수록 정면임
			float FacingFactor = FVector::DotProduct(CameraDirection, ToTargetVector);

			if (FacingFactor < MinimumDotProductThreshold)
			{
				continue; //각도가 지정한 것보다 크면(코사인 값이 지정한 것보다 작으면) 패스
			}
			//거리 정규화 (0~1)
			float NormalizeDist = 1.f - (TargetDistance / MaxLockOnDistance);
			//가중치를 곱한 점수 산출
			float TargetWeight = (FacingFactor * DotWeight) + NormalizeDist * DistWeight;

			if (TargetWeight > HighestWeight) //최고점수 갱신
			{
				HighestWeight = TargetWeight;
				BestTarget = Target;
			}
		}

		if (BestTarget)
		{
			CurrentLockOnTarget = BestTarget;
		}

		//TODO: 캐릭터 bOrient 상태 전환 함수 호출 
	}
	else
	{
		return nullptr;
	}
}

void UVGLockOnComponent::LockOnPerform()
{
	IVGCharacterGameplayTagEditor* TagEditor = Cast<IVGCharacterGameplayTagEditor>(GetOwner());
	//아직은 락온 전환 기능은 없습니다. 어려워잉
	// 이미 락온 중이라면 해제
	if (CurrentLockOnTarget)
	{
		CurrentLockOnTarget = nullptr;
		OnLockOnTargetChanged.Broadcast(nullptr);
		TagEditor->RemoveGameplayTag(VigilantCharacter::LockOn);
		return;
	}

	// 새로운 타겟 탐색
	if (AActor* NewTarget = FindBestTarget())
	{
		CurrentLockOnTarget = NewTarget;
		TagEditor->AddGameplayTag(VigilantCharacter::LockOn);
		OnLockOnTargetChanged.Broadcast(NewTarget);
	}
}


// Called every frame
void UVGLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
