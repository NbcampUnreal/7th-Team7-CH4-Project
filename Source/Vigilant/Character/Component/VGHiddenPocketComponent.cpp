// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHiddenPocketComponent.h"
#include "VGEquipmentComponent.h"
#include "Character/VGCharacterBase.h"
#include "Common/VGGameplayTags.h"
#include "Components/PrimitiveComponent.h"
#include "Core/VGPlayerState.h"
#include "Equipment/VGEquippableActor.h"
#include "Net/UnrealNetwork.h"

UVGHiddenPocketComponent::UVGHiddenPocketComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;

}

void UVGHiddenPocketComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UVGHiddenPocketComponent, HiddenItem);
}

void UVGHiddenPocketComponent::TogglePocket()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	AVGPlayerState* PlayerState = OwnerPawn->GetPlayerState<AVGPlayerState>();
	if (!PlayerState) return;
	
	// 마피아 태그가 없다면 아무 동작도 하지 않고 바로 종료
	if (!PlayerState->HasPlayerTag(VigilantRoleTags::Mafia))
	{
		return; 
	}
	
	AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(OwnerPawn);
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
		{
			UE_LOG(LogTemp, Warning, TEXT("몽타주 재생 중 주머니 사용 취소"));
			return;
		}
	}
	
	// 이미 주머니에 무언가 있다면 -> 버리기 로직
	if (HiddenItem != nullptr)
	{
		Server_DropHiddenItem();
		UE_LOG(LogTemp, Warning, TEXT("주머니에서 아이템을 버렸습니다"));
	}
	// 주머니가 비어있다면 -> 줍기(숨기기) 로직
	else
	{
		// VGEquipmentComponent에서 현재 하이라이트된 타겟 가져오기
		UVGEquipmentComponent* EquipComp = GetOwner()->GetComponentByClass<UVGEquipmentComponent>();
		if (EquipComp)
		{
			AActor* TargetActor = EquipComp->GetCurrentInteractableTarget();
            
			// 손으로 들 수 있는 물건(EquippableActor)인지 검사
			AVGEquippableActor* EquippableTarget = Cast<AVGEquippableActor>(TargetActor);
			if (EquippableTarget)
			{
				Server_StashItem(EquippableTarget);
				UE_LOG(LogTemp, Warning, TEXT("아이템을 주머니에 숨겼습니다"));
			}
		}
	}
}

void UVGHiddenPocketComponent::OnRep_HiddenItem(AVGEquippableActor* OldItem)
{
	// 클라이언트 측 시각적 동기화
	if (HiddenItem)
	{
		HandlePocketState(HiddenItem, true);
		
		// 클라이언트용 UI 방송 (아이템 획득)
		if (HiddenItem->EquipmentData)
		{
			OnPocketItemStashed.Broadcast(HiddenItem->EquipmentData, HiddenItem->GetItemMesh());
		}
	}
	else if (OldItem)
	{
		HandlePocketState(OldItem, false);
		
		// 클라이언트용 UI 방송 (아이템 버림)
		OnPocketItemDropped.Broadcast();
	}
}

void UVGHiddenPocketComponent::HandlePocketState(AVGEquippableActor* Item, bool bIsHiding)
{
	if (!Item) return;

	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(Item->GetRootComponent());

	if (bIsHiding)
	{
		// 물리 끄기 및 충돌 무시
		if (RootComp)
		{
			RootComp->SetSimulatePhysics(false);
			RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		// 화면에서 안 보이게 숨기기
		Item->SetActorHiddenInGame(true);
		// 캐릭터에 부착
		Item->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		// 캐릭터에서 분리
		Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// 화면에 다시 보이게 설정
		Item->SetActorHiddenInGame(false);
		// 물리 켜기 및 충돌 복구 (땅으로 떨어지게)
		if (RootComp)
		{
			RootComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			RootComp->SetSimulatePhysics(true);
		}
	}
}

void UVGHiddenPocketComponent::Server_DropHiddenItem_Implementation()
{
	if (HiddenItem == nullptr) return;

	// 레이저를 쏠 시작점(가슴 높이)과 끝점(100유닛 앞) 설정
	FVector StartLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 50.0f); 
	FVector ForwardVector = GetOwner()->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 100.0f);

	FVector DropLocation = EndLocation; // 기본적으로는 100유닛 앞에 버림

	// Line Trace(레이저) 세팅
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner()); // 플레이어 몸체는 통과하도록 무시
	QueryParams.AddIgnoredActor(HiddenItem); // 버려질 아이템 본체도 무시

	// 레이저를 쏴서 시야(ECC_Visibility)에 걸리는 벽이 있는지 검사
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		// 부딪힌 지점(ImpactPoint)에서 벽이 바라보는 방향(ImpactNormal)으로 살짝(15유닛) 밀어내서 바닥에 떨어뜨림
		DropLocation = HitResult.ImpactPoint + (HitResult.ImpactNormal * 15.0f);
	}

	// 최종 계산된 안전한 위치로 아이템 이동 및 물리 켜기
	HiddenItem->SetActorLocation(DropLocation);
	HandlePocketState(HiddenItem, false);
	HiddenItem = nullptr;
	
	// UI 방송 (아이템 버림)
	OnPocketItemDropped.Broadcast();
}

void UVGHiddenPocketComponent::Server_StashItem_Implementation(AVGEquippableActor* ItemToStash)
{
	if (!ItemToStash || HiddenItem != nullptr) return;

	// 거리 검증
	float Distance = FVector::Distance(GetOwner()->GetActorLocation(), ItemToStash->GetActorLocation());
	if (Distance > 300.0f) return;

	// 누군가 이미 장착했거나 숨겼는지 검사
	if (ItemToStash->GetAttachParentActor() != nullptr) return;

	HiddenItem = ItemToStash;
	HandlePocketState(HiddenItem, true);
	
	// UI 방송 (아이템 획득)
	if (HiddenItem && HiddenItem->EquipmentData)
	{
		OnPocketItemStashed.Broadcast(HiddenItem->EquipmentData, HiddenItem->GetItemMesh());
	}
}
