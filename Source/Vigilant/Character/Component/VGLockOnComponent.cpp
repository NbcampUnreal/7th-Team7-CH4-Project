// Fill out your copyright notice in the Description page of Project Settings.


#include "VGLockOnComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UVGLockOnComponent::UVGLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	TargetClassFilter = APawn::StaticClass();
}

void UVGLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UVGLockOnComponent::IsTargetObscured(const FVector& StartLocation, const FVector& EndLocation, AActor* TargetToIgnore) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return true;
	}

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (TargetToIgnore)
	{
		Params.AddIgnoredActor(TargetToIgnore);
	}

	return GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Visibility,
		Params
	);
}

void UVGLockOnComponent::CheckTargetLineOfSight(FVector StartLocation, FVector EndLocation, float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	bool bHit = IsTargetObscured(StartLocation, EndLocation, CurrentLockOnTarget);
	if (bHit)
	{
		CurrentOcclusionTime += DeltaTime;
		if (CurrentOcclusionTime > MaxOcclusionTime)
		{
			ClearLockOn();
		}
	}
	else
	{
		CurrentOcclusionTime = 0.f;
	}
}

AActor* UVGLockOnComponent::FindBestTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	UCameraComponent* FollowCamera = Owner->FindComponentByClass<UCameraComponent>();
	if (!FollowCamera)
	{
		return nullptr;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreTargets;
	IgnoreTargets.Add(Owner);

	TArray<AActor*> OverlappedTargets;
	bool bResult = UKismetSystemLibrary::SphereOverlapActors(
		Owner,
		Owner->GetActorLocation(),
		MaxLockOnDistance,
		ObjectTypes,
		TargetClassFilter,
		IgnoreTargets,
		OverlappedTargets);

	if (!bResult)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float HighestWeight = -1.0f;

	FVector CameraLocation = FollowCamera->GetComponentLocation();
	FVector CameraDirection = FollowCamera->GetForwardVector();
	FVector OwnerLocation = Owner->GetActorLocation();

	for (AActor* Target : OverlappedTargets)
	{
		FVector TargetLocation = Target->GetActorLocation();
		FVector ToTargetVector = (TargetLocation - CameraLocation).GetSafeNormal();
		float TargetDistance = FVector::Dist(CameraLocation, TargetLocation);
		float FacingFactor = FVector::DotProduct(CameraDirection, ToTargetVector);

		if (FacingFactor < MinimumDotProductThreshold)
		{
			continue;
		}
		if (IsTargetObscured(OwnerLocation, TargetLocation, Target))
		{
			continue;
		}

		float NormalizeDist = 1.f - (TargetDistance / MaxLockOnDistance);
		float TargetWeight = (FacingFactor * DotWeight) + (NormalizeDist * DistWeight);

		if (TargetWeight > HighestWeight)
		{
			HighestWeight = TargetWeight;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

void UVGLockOnComponent::LockOnPerform()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	if (CurrentLockOnTarget)
	{
		ClearLockOn();
		return;
	}

	if (AActor* NewTarget = FindBestTarget())
	{
		CurrentLockOnTarget = NewTarget;

		if (LockOnWidgetClass)
		{
			if (!LockOnWidgetInstance)
			{
				LockOnWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), LockOnWidgetClass);
			}

			if (LockOnWidgetInstance)
			{
				LockOnWidgetInstance->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
				LockOnWidgetInstance->AddToViewport();
			}
		}
		OnLockOnTargetChanged.Broadcast(NewTarget);
	}
}

void UVGLockOnComponent::ClearLockOn()
{
	if (CurrentLockOnTarget)
	{
		CurrentLockOnTarget = nullptr;
		OnLockOnTargetChanged.Broadcast(nullptr);
	}
	if (LockOnWidgetInstance)
	{
		LockOnWidgetInstance->RemoveFromParent();
	}
	CurrentOcclusionTime = 0.0f;
}


void UVGLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentLockOnTarget || !IsValid(CurrentLockOnTarget))
	{
		if (CurrentLockOnTarget)
		{
			ClearLockOn();
		}
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	FVector StartLocation = OwnerPawn->GetActorLocation();
	FVector TargetLocation = CurrentLockOnTarget->GetActorLocation();
	TargetLocation.Z += LockOnTargetZOffset;

	// 거리 초과 시 락온 해제
	if (FVector::Dist(StartLocation, TargetLocation) > TargetLostDistance)
	{
		ClearLockOn();
		return;
	}

	// 장애물 차단 체크 (내부에서 ClearLockOn 호출 가능)
	CheckTargetLineOfSight(StartLocation, TargetLocation, DeltaTime);
	if (!CurrentLockOnTarget)
	{
		return;
	}

	// 카메라 회전 보간
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
	FRotator CurrentRotation = PlayerController->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CameraInterpSpeed);
	PlayerController->SetControlRotation(NewRotation);

	// 락온 마커 UI 위치 갱신
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		FVector2D ScreenPosition;
		if (UGameplayStatics::ProjectWorldToScreen(PlayerController, TargetLocation, ScreenPosition))
		{
			LockOnWidgetInstance->SetPositionInViewport(ScreenPosition);
		}
	}
}
