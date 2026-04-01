// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
/**
 * 
/*
 필요한 태그를 미리 선언	
 사용법 네임스페이스::설정한이름 VigilantTags::Sprint
 
 정의방법 h에서 캐싱할 이름, cpp에서 태그 등록
*/
namespace VigilantTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
}


class VIGILANT_API VGGameplayTags
{
public:
	VGGameplayTags();
	~VGGameplayTags();
};
