// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHUDWidget.h"

#include "GameplayTagContainer.h"
#include "Common/VGGameplayTags.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"

void UVGHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddUniqueDynamic(this, &UVGHUDWidget::OnReadyButtonClicked);
		
	}
}

void UVGHUDWidget::SetEquipIcon(int32 SlotIndex, UTexture2D* IconTexture)
{
	UImage* TargetImage = (SlotIndex == 1) ? Equip_Left : Equip_Right; // 왼손 오른손 판단
	if (TargetImage && IconTexture)
	{
		TargetImage->SetBrushFromTexture(IconTexture);
		TargetImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f)); // 투명도 복구
	}
}

void UVGHUDWidget::ClearEquipIcon(int32 SlotIndex)
{
	UImage* TargetImage = (SlotIndex == 1) ? Equip_Left : Equip_Right;
	if (TargetImage)
	{
		// 이미지를 투명하게 만들어 숨김
		TargetImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UVGHUDWidget::UpdateStaminaUI(float NewValue, float MaxValue)
{
	if (MaxValue > 0.f)
	{
		StaminaBar->SetPercent(NewValue/MaxValue);
	}
}

void UVGHUDWidget::UpdateHealthUI(float NewValue, float MaxValue)
{
	if (BloodImage)
	{
		if (MaxValue > 0.f)
		{
			float HealthRatio = NewValue/MaxValue;
			float TargetAlpha = 0.5f - HealthRatio*0.5f;

			// 체력 20% 부터 표시됨
			if (NewValue/MaxValue <= 0.2f)
			{
				BloodImage->SetVisibility(ESlateVisibility::Visible);
				BloodImage->SetRenderOpacity(TargetAlpha);
			}
			else
			{
				BloodImage->SetVisibility(ESlateVisibility::Collapsed);
				BloodImage->SetRenderOpacity(0.f);
			}
		}
	}
}

void UVGHUDWidget::UpdateMissionUI(float NewValue, float MaxValue)
{
}

void UVGHUDWidget::ChangeSelectedEquipSlot(int32 SlotIndex)
{
	if (GlowingFrame &&NormalFrame)
	{
		if (SlotIndex == 1) // 왼손 선택 상태면
		{
			Equip_Left_Frame->SetBrushFromTexture(GlowingFrame);
			Equip_Right_Frame->SetBrushFromTexture(NormalFrame);
		}
		else if (SlotIndex == 2) //오른손 선택 상태면
		{
			Equip_Right_Frame->SetBrushFromTexture(GlowingFrame);
			Equip_Left_Frame->SetBrushFromTexture(NormalFrame);
		}
		else
		{
			// 미래에 마피아 3번째 슬롯이 생긴다면 여기에 작성	
		}
	}
}
void UVGHUDWidget::SetPhaseTimeData(float InStartTime, float InEndTime, bool Init)
{
	TargetStartTime = InStartTime;
	if (Init == true)
	{
		TargetNewEndTime = InEndTime;
		TargetOldEndTime = TargetNewEndTime;
	}
	if (!FMath::IsNearlyEqual(TargetNewEndTime, InEndTime))
	{
		TargetOldEndTime = TargetNewEndTime;
	}
	TargetNewEndTime = InEndTime;
	
	
	if (GetWorld() && TargetNewEndTime > TargetStartTime)
	{
		
		GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
		
		
		GetWorld()->GetTimerManager().SetTimer(
			PhaseTimerHandle, 
			this, 
			&UVGHUDWidget::UpdateTimePerSecond, 
			0.5f, 
			true
		);
		
		// 타이머 시작과 동시에 0초 차 지연 없이 UI를 즉시 1회 반영
		UpdateTimePerSecond();
	}
}

void UVGHUDWidget::StopPhaseTimeData()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
	}
}


void UVGHUDWidget::UpdateTimePerSecond()
{
	if (!GetWorld() || !MissionProgress || !TimerBarSize) return;

	AGameStateBase* BaseGameState = GetWorld()->GetGameState();
	float CurrentTime = BaseGameState ? BaseGameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	float TotalTime = TargetNewEndTime - TargetStartTime;
	float OldTotalTime = TargetOldEndTime - TargetStartTime;
	float ElapsedTime = CurrentTime - TargetStartTime;

	if (TotalTime > 0.f)
	{
		float MissionTimeRatio = FMath::Clamp(ElapsedTime / TotalTime, 0.0f, 1.0f);
		
		// 프로그레스 바 업데이트
		MissionProgress->SetPercent(MissionTimeRatio);

		float OffsetSizeRaito = OldTotalTime/TotalTime;
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerBarSize->Slot))
		{
			FMargin CurrentOffsets = CanvasSlot->GetOffsets();
			CurrentOffsets.Right = 360.0f * OffsetSizeRaito;
			CanvasSlot->SetOffsets(CurrentOffsets);
		}

		
		if (CurrentTime >= TargetNewEndTime)
		{
			GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
		}
	}
}

void UVGHUDWidget::PauseUpdateTimer()
{
	GetWorld()->GetTimerManager().PauseTimer(PhaseTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("[HUD] 미션 페이즈 타이머 일시정지"));

	
}

void UVGHUDWidget::ResumeUpdateTimer()
{
	GetWorld()->GetTimerManager().UnPauseTimer(PhaseTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("[HUD] 미션 페이즈 타이머 재개"));
}

void UVGHUDWidget::OnReadyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("버튼 누름"));
	OnReadyDelegate.Broadcast(true);
}

void UVGHUDWidget::DisplayRole(FGameplayTag RoleTag)
{

	
	// 숨겨져 있던 텍스트를 다시 보이게 만듬, 보이긴 하지만 클릭이 안되는 비저빌리티
	RoleText->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 2. 태그에 따라 텍스트 및 색상 세팅
	if (RoleTag == VigilantRoleTags::Mafia)
	{
		RoleText->SetText(FText::FromString(TEXT("제물의 도주를 방해하고 \n 정체를 숨겨라")));
		
		RoleText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		if (HiddenInven)
		{
			HiddenInven->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else if (RoleTag == VigilantRoleTags::Citizen)
	{
		RoleText->SetText(FText::FromString(TEXT("비밀을 풀고 \n 숨어있는 무언가를 찾아내라.")));
		RoleText->SetColorAndOpacity(FSlateColor(FLinearColor::White)); 
	}

	// 0에서 1로 변하는 페이드인 애니메이션 강제 재생
	if (RoleAnim)
	{
		PlayAnimation(RoleAnim);
	}

	// 타이머 세팅 (3초 뒤에 HideRoleText 함수 실행)
	if (GetWorld())
	{
		// 기존에 돌고 있던 타이머가 있다면 초기화 후 다시 실행
		GetWorld()->GetTimerManager().ClearTimer(RoleTextTimerHandle);
			
		GetWorld()->GetTimerManager().SetTimer(
			RoleTextTimerHandle, 
			this, 
			&UVGHUDWidget::HideRoleText, 
			3.0f, 
			false // 반복 안 함
		);
	}
}


void UVGHUDWidget::HideRoleText()
{
	if (RoleText)
	{
		//텍스트를 다시 완전히 숨김 처리
		RoleText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
	

