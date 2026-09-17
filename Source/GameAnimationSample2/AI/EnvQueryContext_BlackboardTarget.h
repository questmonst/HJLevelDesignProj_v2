// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_BlackboardTarget.generated.h"

// EQS 컨텍스트: 질의자(적)의 블랙보드 TargetActor를 돌려준다.
//
// 엄폐물 찾기 EQS는 "타겟(미카)에게서 가려지는 지점"을 골라야 하는데, EQS 기본 컨텍스트는
// Querier(자기 자신)뿐이라 타겟을 모른다. BP 컨텍스트로 만들면 Cast 배선이 필요해 C++로 둔다.
// 다른 키를 쓰고 싶으면 이 클래스를 BP로 상속해 TargetKeyName만 바꾼다.
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API UEnvQueryContext_BlackboardTarget : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Context", meta=(ToolTip="타겟 액터가 들어 있는 블랙보드 키 이름"))
	FName TargetKeyName = TEXT("TargetActor");
};
