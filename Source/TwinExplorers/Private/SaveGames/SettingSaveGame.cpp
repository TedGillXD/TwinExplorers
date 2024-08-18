// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGames/SettingSaveGame.h"

USettingSaveGame::USettingSaveGame() {
	MasterVolume = 1.f;
	EffectVolume = 1.f;
	BGMVolume = 1.f;

	IpAndPort = "127.0.0.1:7777";
}
