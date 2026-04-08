#include "VGMissionSandbag.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGStatComponent.h"
#include "Net/UnrealNetwork.h"

AVGMissionSandbag::AVGMissionSandbag()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("Pawn"));
	
	StatComponent = CreateDefaultSubobject<UVGStatComponent>(TEXT("StatComponent"));
}

void AVGMissionSandbag::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, CurrentHPRatio);
}

void AVGMissionSandbag::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		StatComponent->OnDead.AddDynamic(this, &AVGMissionSandbag::OnDead);
		StatComponent->OnHPChanged.AddDynamic(this, &AVGMissionSandbag::OnHPChanged);
	}
}

void AVGMissionSandbag::RegisterAttacker(AVGCharacterBase* Attacker)
{
	if (!HasAuthority()) return;
	if (!Attacker) return;

	// 데미지를 줄 때마다 마지막 공격자 갱신
	LastAttacker = Attacker;
}

void AVGMissionSandbag::OnHPChanged(float NewHP, float MaxHP)
{
	// HP 비율을 Replicate해서 클라이언트 UI에 표시
	CurrentHPRatio = (MaxHP > 0.f) ? (NewHP / MaxHP) : 0.f;
}

void AVGMissionSandbag::OnRep_CurrentHPRatio()
{
	// TODO: 위젯 업데이트 로직 or 제거
	UE_LOG(LogTemp, Display, TEXT("CurrentHPRatio is %f"), CurrentHPRatio);
}

void AVGMissionSandbag::OnDead(AController* LastInstigator)
{
	// 막타 플레이어와 함께 미션에 보고
	OnSandbagDefeated.Broadcast(LastAttacker);
	UE_LOG(LogTemp, Display, TEXT("[%s] is dead."), *GetName());
}
