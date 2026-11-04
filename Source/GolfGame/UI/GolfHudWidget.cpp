#include "UI/GolfHudWidget.h"
#include "UI/GolfUIManager.h"
#include "Systems/CourseManager.h"
#include "Systems/DayCycleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

static UDayCycleSubsystem* GetDayCycleFor(const UUserWidget* widget)
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(widget);
	return gameInstance ? gameInstance->GetSubsystem<UDayCycleSubsystem>() : nullptr;
}

static UCourseManager* GetCourseFor(const UUserWidget* widget)
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(widget);
	return gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
}

void UGolfHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGolfUIManager* ui = gameInstance->GetSubsystem<UGolfUIManager>())
		{
			ui->RegisterHud(this);
		}
	}

	if (UDayCycleSubsystem* dayCycle = GetDayCycleFor(this))
	{
		dayCycle->onPhaseChanged.AddDynamic(this, &UGolfHudWidget::HandlePhaseChanged);
		dayCycle->onDayEnded.AddDynamic(this, &UGolfHudWidget::HandleDayEnded);
	}
	if (UCourseManager* course = GetCourseFor(this))
	{
		course->onCashChanged.AddDynamic(this, &UGolfHudWidget::HandleCashChanged);
	}

	OnStateChanged();
}

void UGolfHudWidget::NativeDestruct()
{
	if (UDayCycleSubsystem* dayCycle = GetDayCycleFor(this))
	{
		dayCycle->onPhaseChanged.RemoveDynamic(this, &UGolfHudWidget::HandlePhaseChanged);
		dayCycle->onDayEnded.RemoveDynamic(this, &UGolfHudWidget::HandleDayEnded);
	}
	if (UCourseManager* course = GetCourseFor(this))
	{
		course->onCashChanged.RemoveDynamic(this, &UGolfHudWidget::HandleCashChanged);
	}

	Super::NativeDestruct();
}

FText UGolfHudWidget::GetDayText() const
{
	const UDayCycleSubsystem* dayCycle = GetDayCycleFor(this);
	return FText::FromString(FString::Printf(TEXT("Day: %d"), dayCycle ? dayCycle->GetCurrentDay() : 0));
}

FText UGolfHudWidget::GetCashText() const
{
	const UCourseManager* course = GetCourseFor(this);
	return FText::FromString(FString::Printf(TEXT("Cash: $%d"), course ? course->GetPlayerCash() : 0));
}

FText UGolfHudWidget::GetPhaseText() const
{
	const UDayCycleSubsystem* dayCycle = GetDayCycleFor(this);
	if (!dayCycle)
	{
		return FText::GetEmpty();
	}
	switch (dayCycle->GetCurrentPhase())
	{
	case EDayPhase::MorningPrep: return FText::FromString(TEXT("Closed - Prep"));
	case EDayPhase::OpenHours:   return FText::FromString(TEXT("Open"));
	default:                     return FText::FromString(TEXT("Closed"));
	}
}

FText UGolfHudWidget::GetTimeText() const
{
	const UDayCycleSubsystem* dayCycle = GetDayCycleFor(this);
	return FText::FromString(FString::Printf(TEXT("Time: %s"), dayCycle ? *dayCycle->GetFormattedTime() : TEXT("--")));
}

FText UGolfHudWidget::GetCloseTimerText() const
{
	const UDayCycleSubsystem* dayCycle = GetDayCycleFor(this);
	return FText::FromString(FString::Printf(TEXT("Close Time: %d"), dayCycle ? FMath::RoundToInt(dayCycle->GetCloseElapsedMinutes()) : 0));
}

void UGolfHudWidget::HandlePhaseChanged(EDayPhase newPhase)
{
	OnStateChanged();
}

void UGolfHudWidget::HandleDayEnded()
{
	OnStateChanged();
}

void UGolfHudWidget::HandleCashChanged(int32 newCash)
{
	OnStateChanged();
}
