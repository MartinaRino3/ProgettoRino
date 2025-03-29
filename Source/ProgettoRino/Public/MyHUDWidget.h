#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MyHUDWidget.generated.h"


UCLASS()
class PROGETTORINO_API UMyHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnTapSpawnBrawler();

	UFUNCTION()
	void OnTapSpawnSniper();

	UFUNCTION()
	void OnTapEndTurn();

	UFUNCTION()
	void RemoveEndTurn();

	UFUNCTION()
	void ResetGame();
	
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SpawnBrawler;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SpawnSniper;

	UPROPERTY(meta = (BindWidget))
	UButton* ButtonEndTurno;

	UPROPERTY(meta = (BindWidget))
	UButton* Reset;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TurnText;


protected:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

};
