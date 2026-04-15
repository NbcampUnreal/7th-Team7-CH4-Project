// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "VGLockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVGLockOnComponent();
	
	AActor* GetCurrentLockOnTarget() const {return CurrentLockOnTarget;}
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void ToggleLockOn();
	
	
	
	//락온 기능
	UPROPERTY()
	TArray<AActor*> LockOnTargetList;
	
	TObjectPtr<UUserWidget> LockOnWidgetInstance;
	


	
	UPROPERTY(EditAnywhere, Category = "LockOn")
	TSubclassOf<APawn> TargetClassFilter;
	
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float CameraInterpSpeed = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MaxLockOnDistance = 2000.0f;
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float TargetLostDistance = 2500.f;
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MinimumDotProductThreshold = 0.5;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float DotWeight = 0.7f;
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float DistWeight = 0.3f;
	UPROPERTY()
	USpringArmComponent* CachedSpringArm;

private:
	AActor* FindBestTarget();
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentLockOnTarget;
	

public:
	void LockOnPerform();
	
	FOnLockOnTargetChanged OnLockOnTargetChanged;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
