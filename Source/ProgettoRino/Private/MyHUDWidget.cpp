// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDWidget.h"
#include "MyGameModeBase.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "AISmart.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"


bool UMyHUDWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
		return false;

	return true;
}

void UMyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//Add the dinamic to each button 
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMyHUDWidget::OnPlayButtonClicked);
	}
	if (SpawnBrawler)
	{
		SpawnBrawler->OnClicked.AddDynamic(this, &UMyHUDWidget::OnTapSpawnBrawler);
	}
	if (SpawnSniper)
	{
		SpawnSniper->OnClicked.AddDynamic(this,&UMyHUDWidget::OnTapSpawnSniper);
	}
	if (ButtonEndTurno)
	{
		ButtonEndTurno->OnClicked.AddDynamic(this, &UMyHUDWidget::OnTapEndTurn);
	}
	if (Reset)
	{
		Reset->OnClicked.AddDynamic(this,&UMyHUDWidget::ResetGame);
	}
}


void UMyHUDWidget::OnPlayButtonClicked()
{
	AMyGameModeBase* GameMode  = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	//Remove the button 
	StartButton->RemoveFromParent();

	//Appear the other 
	SpawnBrawler->SetVisibility(ESlateVisibility::Visible);
	SpawnSniper->SetVisibility(ESlateVisibility::Visible);
	ButtonEndTurno->SetVisibility(ESlateVisibility::Visible);
	TurnText->SetVisibility(ESlateVisibility::Visible);
	Reset->SetVisibility(ESlateVisibility::Visible);
	
	//Start the game
	GameMode->ChoosePlayerAndStartGame();
}


void UMyHUDWidget::OnTapSpawnBrawler()
{	
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->IsSpawnBrawler();
	SpawnBrawler->SetVisibility(ESlateVisibility::Collapsed);
}

void UMyHUDWidget::OnTapSpawnSniper()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->IsSpawnSniper();
	SpawnSniper->SetVisibility(ESlateVisibility::Collapsed);
}

void UMyHUDWidget::OnTapEndTurn()
{;
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->HumanPlayer->EndPlayerTurn();
}

void UMyHUDWidget::RemoveEndTurn()
{
	ButtonEndTurno->SetVisibility(ESlateVisibility::Collapsed);
}

void UMyHUDWidget::ResetGame()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
		return;
	
	AGameField* Field = GameMode->GField;
	if (!Field)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	if (GameMode->CurrentPlayer == 1)
		return;

	if (GameMode->HumanPlayer)
	{
		if (GameMode->HumanPlayer->bTimerFinished == false)
		{
			return;
		}
		if (IsValid(GameMode->HumanPlayer->BrawlerUnit)) 
		{
			GameMode->HumanPlayer->BrawlerUnit->Destroy();
		}
		if (IsValid(GameMode->HumanPlayer->SniperUnit)) 
		{
			GameMode->HumanPlayer->SniperUnit->Destroy();
		}
		GameMode->HumanPlayer->ResetHuman();
	}

	if (GameMode->RandomPlayer)
	{
		if (GameMode->RandomPlayer->bTimerFinished == false)
		{
			return;
		}
		if (IsValid(GameMode->RandomPlayer->BrawlerUnit)) 
		{
			GameMode->RandomPlayer->BrawlerUnit->Destroy();
		}
		if (IsValid(GameMode->RandomPlayer->SniperUnit)) 
		{
			GameMode->RandomPlayer->SniperUnit->Destroy();		
		}
		GameMode->RandomPlayer->ResetAI();
	}
	
	TArray<AActor*> ActorsToDestroy;
	UGameplayStatics::GetAllActorsOfClass(World, ATile::StaticClass(), ActorsToDestroy);

	for (AActor* Actor : ActorsToDestroy)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	ActorsToDestroy.Empty();

	UGameplayStatics::GetAllActorsOfClass(World, ATileObstacle::StaticClass(), ActorsToDestroy);

	for (AActor* Actor : ActorsToDestroy)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	
	Field->TileArray.Empty();
	Field->TileMap.Empty();

	Field->GenerateField();

	if (SpawnBrawler)
		SpawnBrawler->SetVisibility(ESlateVisibility::Visible);
	if (SpawnSniper)
		SpawnSniper->SetVisibility(ESlateVisibility::Visible);

	GameMode->MoveBox->MoveHistory->ClearChildren();

	GameMode->GameInstance->UpdateLifes(nullptr,nullptr);

	GameMode->ChoosePlayerAndStartGame();
}
