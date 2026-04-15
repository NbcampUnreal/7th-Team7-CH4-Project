// Fill out your copyright notice in the Description page of Project Settings.


#include "VGLockOnComponent.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "GameplayTagAssetInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
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
		UE_LOG(LogTemp, Warning, TEXT("오너를 못찾앗다"));
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
	
	DrawDebugSphere(
	GetWorld(),
	Owner->GetActorLocation(),
	MaxLockOnDistance,
	32,
	FColor::Red,
	false,
	3.0f
);
	
	UE_LOG(LogTemp, Warning, TEXT("오너 위치: %s, 탐색 반경: %f"), 
	*Owner->GetActorLocation().ToString(), MaxLockOnDistance);
	
	bool bResult =
		UKismetSystemLibrary::SphereOverlapActors(
			Owner,
			Owner->GetActorLocation(),
			MaxLockOnDistance,
			ObjectTypes,
			nullptr,
			IgnoreTargets,
			LockOnTargetList);

	if (bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("오버랩 결과: %s, 찾은 액터 수: %d"), 
	bResult ? TEXT("성공") : TEXT("실패"), LockOnTargetList.Num());
		
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
			UE_LOG(LogTemp, Warning, TEXT("락온 타겟 설정"));
		}
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("오버랩실패"));
		return nullptr;
	}
	return CurrentLockOnTarget;
}

void UVGLockOnComponent::LockOnPerform()
{

	IVGCharacterGameplayTagEditor* TagEditor = Cast<IVGCharacterGameplayTagEditor>(GetOwner());
	//-------------------------------
	// 1. 함수 호출 확인
	UE_LOG(LogTemp, Warning, TEXT("[LockOnPerform] 1. 컴포넌트 함수 진입 성공"));

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		// 2. 캐스팅 실패 확인
		UE_LOG(LogTemp, Error, TEXT("[LockOnPerform] ❌ 실패: GetOwner()를 APawn으로 캐스팅할 수 없습니다."));
		return;
	}
    
	if (!OwnerPawn->GetController())
	{
		// 3. 컨트롤러(빙의 상태) 확인
		UE_LOG(LogTemp, Error, TEXT("[LockOnPerform] ❌ 실패: [%s] 액터가 GetController() == nullptr 상태입니다. (빙의되지 않음)"), *OwnerPawn->GetName());
		return;
	}
    
	// 4. 모든 방어선 통과 확인
	UE_LOG(LogTemp, Warning, TEXT("[LockOnPerform] 2. [%s] 컨트롤러 검사 통과! 락온 탐색 시작!"), *OwnerPawn->GetName());
	
	// CDO가 아닌 실제 인스턴스인지 확인 및 실행 주체 이름 출력
	if (!OwnerPawn->GetController())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 컨트롤러 없음 - 해당 액터에서 락온 실행 거부됨"), *OwnerPawn->GetName());
		return;
	}
	
	// 💡 3. 실제 인스턴스가 로직에 정상 진입했는지 확인
	UE_LOG(LogTemp, Warning, TEXT("[%s] 실제 인스턴스 락온 로직 진입 성공!"), *OwnerPawn->GetName());
	//-------------------------------
	
	UE_LOG(LogTemp, Warning, TEXT("OwnerPawn: %s"), 
		OwnerPawn ? *OwnerPawn->GetName() : TEXT("nullptr"));
    
	if (OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller: %s"), 
			OwnerPawn->GetController() ? *OwnerPawn->GetController()->GetName() : TEXT("nullptr"));
		UE_LOG(LogTemp, Warning, TEXT("IsLocallyControlled: %s"), 
			OwnerPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"));
		UE_LOG(LogTemp, Warning, TEXT("HasAuthority: %s"), 
			OwnerPawn->HasAuthority() ? TEXT("true") : TEXT("false"));
	}
	
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("락온 동작"));
	//아직은 락온 전환 기능은 없습니다. 어려워잉
	// 이미 락온 중이라면 해제
	if (CurrentLockOnTarget)
	{
		CurrentLockOnTarget = nullptr;
		OnLockOnTargetChanged.Broadcast(nullptr);
		TagEditor->RemoveGameplayTag(VigilantCharacter::LockOn);
		UE_LOG(LogTemp, Warning, TEXT("락온 해제"));
		return;
	}

	// 새로운 타겟 탐색
	if (AActor* NewTarget = FindBestTarget())
	{
		CurrentLockOnTarget = NewTarget;
		TagEditor->AddGameplayTag(VigilantCharacter::LockOn);
		UE_LOG(LogTemp, Warning, TEXT("락온 온"));
		OnLockOnTargetChanged.Broadcast(NewTarget);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("타겟을 찻지 못했습니다."));
	}
}


// Called every frame
void UVGLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1. 타겟이 없으면 회전 로직 실행 안 함
	if (!CurrentLockOnTarget) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn) return;

	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;

	// 2. 위치 데이터 추출
	FVector StartLocation = OwnerPawn->GetActorLocation(); // 혹은 카메라 위치
	FVector TargetLocation = CurrentLockOnTarget->GetActorLocation();
	
	
	TargetLocation.Z += 50.0f; 

	// 3. 목표 회전값 계산
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

	// 4. 현재 회전값에서 목표 회전값으로 부드럽게 보간(Lerp)
	FRotator CurrentRotation = Controller->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CameraInterpSpeed);

	// 5. 컨트롤러에 새로운 회전값 적용
	Controller->SetControlRotation(NewRotation);
	// ...
}
