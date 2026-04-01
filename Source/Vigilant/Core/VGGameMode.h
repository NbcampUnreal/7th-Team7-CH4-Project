#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VGGameMode.generated.h"


UCLASS()
class VIGILANT_API AVGGameMode : public AGameMode
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController *NewPlayer) override;
	
protected:
	UPROPERTY(Transient)
	class UVGPhaseBase* CurrentPhase;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Vigilance|Phase")
	void ChangePhase(TSubclassOf<class UVGPhaseBase> NewPhase);
	
	void AssignRoles();
	
	void CheckWinCondition();
	
	void OnMissionCleared(int32 TimeReducedAmount);
	
	void OnPlayerDeath();
};
