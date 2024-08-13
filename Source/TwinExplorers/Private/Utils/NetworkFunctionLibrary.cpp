// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/NetworkFunctionLibrary.h"

bool UNetworkFunctionLibrary::GetIPAndPortFromString(const FString& Input, FString& OutIP, FString& OutPort) {
	// 从Input中解析IP和Port，合法的格式是 xxx.xxx.xxx.xxx:port，其余为不合法，返回false，解析出来后赋值到OutIP和OutPort中，返回true
	// 正则表达式匹配IP地址和端口号的格式
	const FRegexPattern IPPortPattern(TEXT(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))"));
	FRegexMatcher Matcher(IPPortPattern, Input);

	// 尝试匹配输入字符串
	if (Matcher.FindNext()) {
		OutIP = Matcher.GetCaptureGroup(1);
		OutPort = Matcher.GetCaptureGroup(2);

		// 验证每个部分是否在合法范围内
		TArray<FString> Octets;
		OutIP.ParseIntoArray(Octets, TEXT("."));

		if (Octets.Num() == 4) {
			for (const FString& Octet : Octets) {
				int32 OctetValue = FCString::Atoi(*Octet);
				if (OctetValue < 0 || OctetValue > 255) {
					return false;  // IP 地址中的某个部分不在 0-255 范围内
				}
			}
			return true;  // 匹配成功并且合法
		}
	}

	// 输入不合法
	return false;
}
