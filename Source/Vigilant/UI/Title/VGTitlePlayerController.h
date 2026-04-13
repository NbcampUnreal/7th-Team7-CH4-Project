// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VGTitlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API AVGTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	public:	
	virtual void BeginPlay() override;
	
	void JoinServer(const FString& InIPAddress);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ASUIPlayerController", meta = (AllowPrivateAccess = true))
	TSubclassOf<class UUserWidget> TitleWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ASUIPlayerController", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UUserWidget> TitleWidgetInstance;
};
