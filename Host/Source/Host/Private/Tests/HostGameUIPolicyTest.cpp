// Copyright (c) 2026 Likeon. All Rights Reserved.
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UI/SigilGameUIPolicy.h"
#include "UI/SigilGameUISubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHostGameUIPolicyTest,
	"Host.UI.PolicyInitializesWithoutLocalPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHostGameUIPolicyTest::RunTest(const FString& Parameters)
{
	// 删除配置或指定不可实例化的策略时，真实初始化必须使此用例失败。
	if (!TestNotNull(TEXT("引擎实例存在"), GEngine))
	{
		return false;
	}
	UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
	Instance->AddToRoot();
	Instance->InitializeStandalone(
		MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("HostUIPolicy")),
		GetTransientPackage());
	UWorld* World = Instance->GetWorld();
	TestEqual(TEXT("无本地玩家的 Host 夹具"), Instance->GetLocalPlayers().Num(), 0);
	TestNotNull(TEXT("真实初始化创建 UI 子系统"), Instance->GetSubsystem<USigilGameUISubsystem>());
	USigilGameUIPolicy* Policy = USigilGameUIPolicy::GetGameUIPolicy(Instance);
	if (TestNotNull(TEXT("Host 默认配置必须成功实例化 UI 策略"), Policy))
	{
		TestEqual(TEXT("实例使用 Host 专用原生策略"), Policy->GetClass()->GetPathName(),
			FString(TEXT("/Script/Host.HostGameUIPolicy")));
		TestTrue(TEXT("策略归属当前 UI 子系统"),
			Policy->GetOuter() == Instance->GetSubsystem<USigilGameUISubsystem>());
	}
	Instance->Shutdown();
	if (World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	Instance->RemoveFromRoot();
	return true;
}
#endif
