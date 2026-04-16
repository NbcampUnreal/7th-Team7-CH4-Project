// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VGLockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVGLockOnComponent();
	
	AActor* GetCurrentLockOnTarget() const { return CurrentLockOnTarget; }

	void LockOnPerform();
	void ClearLockOn();

	UPROPERTY(BlueprintAssignable)
	FOnLockOnTargetChanged OnLockOnTargetChanged;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	bool IsTargetObscured(const FVector& StartLocation, const FVector& EndLocation, AActor* TargetToIgnore) const;
	void CheckTargetLineOfSight(FVector StartLocation, FVector EndLocation, float DeltaTime);

	UPROPERTY()
	TObjectPtr<UUserWidget> LockOnWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "LockOn|UI")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	UPROPERTY(EditAnywhere, Category = "LockOn|Exception")
	float MaxOcclusionTime = 1.5f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	TSubclassOf<APawn> TargetClassFilter;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float CameraInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MaxLockOnDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float TargetLostDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MinimumDotProductThreshold = 0.5f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float DotWeight = 0.7f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float DistWeight = 0.3f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float LockOnTargetZOffset = 15.f;

private:
	AActor* FindBestTarget();

	UPROPERTY()
	TObjectPtr<AActor> CurrentLockOnTarget;

	float CurrentOcclusionTime = 0.0f;
};
