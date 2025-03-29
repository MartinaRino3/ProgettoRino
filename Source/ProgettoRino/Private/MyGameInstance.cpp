// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameInstance.h"

#include "AISmart.h"
#include "Brawler.h"
#include "HumanPlayer.h"
#include "Sniper.h"
#include "MyPlayerController.h"
#include "PlayerInterface.h"
#include "RandomPlayer.h"
#include "Components/TextBlock.h"
#include "Slate/SGameLayerManager.h"

FString UMyGameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

void UMyGameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}

//Change lifes in the interface
void UMyGameInstance::UpdateLifes(APawn* Unit, AActor* Owner)
{
	//Cast to know what I have to change and the owner
	ABrawler* Brawler = Cast<ABrawler>(Unit);
	ASniper* Sniper = Cast<ASniper>(Unit);
	
	AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Owner);
	//ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(Owner);
	AAISmart* RandomPlayer = Cast<AAISmart>(Owner);
	
	if (Brawler)
	{
		if (HumanPlayer)
		{
			if (HumanPlayer->GetBrawler()== nullptr)
			{
				BrawlerHumanLife = 0;
			}
			else
			{
				BrawlerHumanLife = HumanPlayer->GetLifeBrawlerHuman();
			}
		}
		else if (RandomPlayer)
		{
			if (RandomPlayer->GetBrawler()== nullptr)
			{
				BrawlerRandomLife = 0;
			}
			else
			{
				BrawlerRandomLife = RandomPlayer->GetLifeBrawlerRandom();
			}
		}
	}
	else if (Sniper)
	{
		if (HumanPlayer)
		{
			if (HumanPlayer->GetSniper() == nullptr)
			{
				SniperHumanLife = 0;
			}
			else
			{
				SniperHumanLife = HumanPlayer->GetLifeSniperHuman();
			}
		}
		else if (RandomPlayer)
		{
			if (RandomPlayer->GetSniper() == nullptr)
			{
				SniperRandomLife = 0;
			}
			else
			{
				SniperRandomLife = RandomPlayer->GetLifeSniperRandom();
			}
		}
	}

	if (Unit == nullptr && Owner == nullptr)
	{
		BrawlerHumanLife = 40;
		SniperHumanLife = 20;
		BrawlerRandomLife = 40;
		SniperRandomLife = 20;
	}
}

int32 UMyGameInstance::GetBrawlerHumanLife()
{
	return BrawlerHumanLife;
}

int32 UMyGameInstance::GetSniperHumanLife()
{
	return SniperHumanLife;
}

int32 UMyGameInstance::GetBrawlerRandomLife()
{
	return BrawlerRandomLife;
}

int32 UMyGameInstance::GetSniperRandomLife()
{
	return SniperRandomLife;
}

