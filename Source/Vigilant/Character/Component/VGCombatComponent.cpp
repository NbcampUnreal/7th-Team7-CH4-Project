#include "Character/Component/VGCombatComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

UVGCombatComponent::UVGCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UVGCombatComponent::TryLightAttack()
{
	// 애니메이션: 즉시 로컬에서 재생
	PerformAttack(false);
	
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_TryAttack(false);
	}
}

void UVGCombatComponent::PerformAttack(bool bIsHeavy)
{
	FString RoleString = (GetOwnerRole() == ROLE_Authority) ? TEXT("SERVER") : TEXT("CLIENT");
	FString AttackType = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
	
	if (GEngine)
	{
		FString LogMsg = FString::Printf(TEXT("[%s] %s 공격 실행!"), *RoleString, *AttackType);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, LogMsg);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] %s 공격 실행!"), *RoleString, *AttackType);
	
	// TODO: Anim Montage 재생
}

void UVGCombatComponent::Server_TryAttack_Implementation(bool bIsHeavy)
{
	PerformAttack(bIsHeavy);
}

bool UVGCombatComponent::Server_TryAttack_Validate(bool bIsHeavy)
{
	return true;
}