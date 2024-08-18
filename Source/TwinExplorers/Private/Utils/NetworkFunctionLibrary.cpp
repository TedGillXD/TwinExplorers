// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/NetworkFunctionLibrary.h"

#include "Characters/MainCharacterBase.h"

bool UNetworkFunctionLibrary::GetIPAndPortFromString(const FString& Input, FString& OutIP, FString& OutPort) {
	// 从Input中解析IP和Port，合法的格式是 xxx.xxx.xxx.xxx:port，其余为不合法，返回false，解析出来后赋值到OutIP和OutPort中，返回true
	const FRegexPattern IPPortPattern(TEXT(R"(([^:]+):(\d+))"));
	FRegexMatcher Matcher(IPPortPattern, Input);

	// 尝试匹配输入字符串
	if (Matcher.FindNext()) {
		OutIP = Matcher.GetCaptureGroup(1);   // 获取IP或域名部分
		OutPort = Matcher.GetCaptureGroup(2); // 获取端口号部分

		return true;  // 匹配成功
	}

	// 输入不合法
	return false;
}

void UNetworkFunctionLibrary::GetKeysAndValuesArrayFromMap(const TMap<AMainCharacterBase*, FString>& InMap,
	TArray<AMainCharacterBase*>& OutControllers, TArray<FString>& OutNames) {

	for(auto [Controller, Name] : InMap) {
		OutControllers.Add(Controller);
		OutNames.Add(Name);
	}
}
