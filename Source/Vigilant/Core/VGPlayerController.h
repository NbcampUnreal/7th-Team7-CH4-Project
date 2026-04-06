#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VGPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class VIGILANT_API AVGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	// Input
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> CameraZoomAction;

	// functions
public:
	
	AVGPlayerController();
	
	virtual void BeginPlay() override;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Vigilant|Lobby")
	void Server_SetReady(bool bReady);
	
};
