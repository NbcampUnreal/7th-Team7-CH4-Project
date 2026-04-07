// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickStatue.generated.h"

// VGMissionGimmickStatue.h
UCLASS()
class VIGILANT_API AVGMissionGimmickStatue : public AVGMissionGimmickBase
{
	GENERATED_BODY()
public:
	AVGMissionGimmickStatue();
	
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_GimmickStateTag() override;

private:
	void RotateToTarget(float DeltaTime);

	// 정답 확인용
	bool IsAtAnswerAngle() const;
	
	UFUNCTION()
	void OnRep_TargetAngle();

public:
	// 한 번 상호작용 시 회전하는 각도 (에디터 지정, 기본 90도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Statue")
	float RotateStep = 90.f;

	// 허용 오차 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Statue")
	float AngleTolerance = 2.f;

protected:
	// 시작 각도 — 리셋용
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gimmick|Statue")
	float InitialAngle = 0.f;

	// 정답 각도 — 에디터 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gimmick|Statue")
	float AnswerAngle = 90.f;

	// 현재 목표 각도 — Replicated
	UPROPERTY(ReplicatedUsing = OnRep_TargetAngle)
	float TargetAngle = 0.f;

	// 회전 속도
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Statue")
	float RotationSpeed = 180.f;
};