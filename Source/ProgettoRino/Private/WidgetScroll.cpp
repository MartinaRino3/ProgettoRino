// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetScroll.h"
#include "Components/TextBlock.h"

//Add the movements to the scroll box
void UWidgetScroll::AddMoveToHistory(const FString& MoveText)
{
	UTextBlock* NewMoveText = NewObject<UTextBlock>(this);
	if (NewMoveText)
	{
		NewMoveText->SetText(FText::FromString(MoveText));
		NewMoveText->Font.Size = 16; 
		MoveHistory->AddChild(NewMoveText);
	}
	MoveHistory->ScrollToEnd();
}

bool UWidgetScroll::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success)
		return false;

	return true;
}