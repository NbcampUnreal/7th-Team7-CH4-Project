// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteSlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetRenderingLibrary.h" // 동적 RT 생성을 위해 필요
#include "Materials/MaterialInstanceDynamic.h" // 동적 머티리얼 생성을 위해 필요



void UVGVoteSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (VoteButton)
	{
		VoteButton->OnClicked.AddDynamic(this, &UVGVoteSlotWidget::OnButtonClicked);
	}
}

void UVGVoteSlotWidget::NativeDestruct()
{
	//TODO DynamicRT 메모리에서 제거
	Super::NativeDestruct();
	
}

void UVGVoteSlotWidget::SetupSlot(int32 InPlayerIndex, USceneCaptureComponent2D* TargetCamera )
{
	PlayerIndex = InPlayerIndex;
	
	if (!BaseMaterial || !TargetCamera) return;

	// 도화지(RT)를 메모리에 즉석 생성! (해상도 256x256)
	UTextureRenderTarget2D* DynamicRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, 256, 256);

	// 물감(Material)의 복사본(MID)을 메모리에 즉석 생성!
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

	if (DynamicRT && DynamicMaterial)
	{
		//복사한 물감의 'RT_Texture' 파라미터에 방금 만든 빈 도화지를 끼워 넣습니다.
		DynamicMaterial->SetTextureParameterValue(FName("RT_Texture"), DynamicRT);

		//내 UI 이미지 위젯에 이 물감을 칠합니다.
		if (PortraitImage)
		{
			PortraitImage->SetBrushFromMaterial(DynamicMaterial);
		}
		TargetCamera->TextureTarget = DynamicRT;
		TargetCamera->CaptureScene();

	}
}

void UVGVoteSlotWidget::SetNickName(const FString& InNickName)
{
	if (NickName)
	{
		NickName->SetText(FText::FromString(InNickName));
	}
}

void UVGVoteSlotWidget::OnButtonClicked()
{
	//버튼이 눌렸을 때 로직 -> 투표수 용호님 도와줘요
	UE_LOG(LogTemp, Warning, TEXT("버튼을 눌렀다 범인은 플레이어 %d 이다 !"), PlayerIndex);
	
	//플레이어 인덱스 전달
	OnVoteSlotClickedDelegate.Broadcast(PlayerIndex);
}
