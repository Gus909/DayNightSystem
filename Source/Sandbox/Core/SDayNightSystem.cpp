// Illia Huskov 2024

#include "Sandbox/Core/SDayNightSystem.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Curves/CurveFloat.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Curves/CurveLinearColor.h"

ASDayNightSystem::ASDayNightSystem()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.03f);
	ActorRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = ActorRootComponent;
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(ActorRootComponent);
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(ActorRootComponent);
	SkyLight->bRealTimeCapture = true;
	SunMoonAxis = CreateDefaultSubobject<USceneComponent>(TEXT("SunMoonAxis"));
	SunMoonAxis->SetupAttachment(ActorRootComponent);
	SunDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunDirectionalLight"));
	SunDirectionalLight->SetupAttachment(SunMoonAxis);
	SunDirectionalLight->bAtmosphereSunLight = true;
	SunDirectionalLight->ForwardShadingPriority = 1;
	SunDirectionalLight->AtmosphereSunLightIndex = 1;
	SunDirectionalLight->bUseTemperature = true;
	MoonDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonDirectionalLight"));
	MoonDirectionalLight->SetupAttachment(SunMoonAxis);
	MoonDirectionalLight->bAtmosphereSunLight = true;
	MoonDirectionalLight->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));
	MoonDirectionalLight->ForwardShadingPriority = 0;
	MoonDirectionalLight->bUseTemperature = true;
	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(ActorRootComponent);
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(ActorRootComponent);
	PostProcess->Settings.AutoExposureMinBrightness = .5f;
	PostProcess->Settings.AutoExposureMaxBrightness = .5f;
	SkySphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphere"));
	SkySphere->SetupAttachment(ActorRootComponent);
	SkySphere->SetRelativeScale3D(FVector(10000.f));
}

void ASDayNightSystem::BeginPlay()
{
	Super::BeginPlay();
	MiddayTime = StartDayTime + ((EndDayTime - StartDayTime) / 2.f);
	(!FMath::IsNearlyZero(TimeMultiplier)) ? PickTimeMultiplier() : SetActorTickEnabled(false);
	(Time > StartDayTime && Time < EndDayTime) ? OnDay() : OnNight();
	SetupSkyMaterial();
}

void ASDayNightSystem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetSunMoonRotation();
	SetupSkyMaterial();
	SetupNightParameters();
	SetupDayParameters();
	(Time > StartDayTime && Time < EndDayTime) ? OnDay() : OnNight();
}

void ASDayNightSystem::SetSunMoonRotation()
{
	FRotator LightAxisRotation = FRotator(0.0f);
	if (bDay)
	{
		LightAxisRotation.Pitch = FMath::GetMappedRangeValueUnclamped(FVector2D(StartDayTime, EndDayTime), FVector2D(180.f, 360.f), Time);
	}
	else
	{
		if (Time > EndDayTime)
		{
			LightAxisRotation.Pitch = FMath::GetMappedRangeValueUnclamped(FVector2D(EndDayTime, 24.f + StartDayTime), FVector2D(0.f, 180.f), Time);
		}
		else
		{
			LightAxisRotation.Pitch = FMath::GetMappedRangeValueUnclamped(FVector2D(EndDayTime, 24.f + StartDayTime), FVector2D(0.f, 180.f), 24.f + Time);
		}
	}
	LightAxisRotation.Yaw = SunLightYawRotation;
	SunMoonAxis->SetWorldRotation(LightAxisRotation);
}

void ASDayNightSystem::SetupNightParameters()
{
	MoonDirectionalLight->LightColor = MoonLightColor;
	MoonDirectionalLight->Temperature = MoonLightTemperature;
	MIDSky->SetScalarParameterValue(FName("MoonBrightness"), MoonLightBrightness);
	MIDSky->SetScalarParameterValue(FName("MoonScale"), MoonScale);
	MIDSky->SetScalarParameterValue(FName("MoonRotation"), MoonRotation);
	MIDSky->SetVectorParameterValue(FName("MoonColor"), MoonColor);
	MoonDirectionalLight->Intensity = MoonLightIntensity;
}

void ASDayNightSystem::SetupDayParameters()
{
	SunDirectionalLight->LightColor = SunLightColor;
	SunDirectionalLight->Temperature = SunLightTemperature;
	SunDirectionalLight->LightSourceAngle = SunLightSourceAngle;
	SunDirectionalLight->Intensity = SunLightIntensity;
}

void ASDayNightSystem::OnNight()
{
	SkyAtmosphere->SetRayleighScattering(NighttimeRaylayScattering);
	SkyAtmosphere->SetMultiScatteringFactor(NighttimeMultiScattering);
	MoonDirectionalLight->SetVisibility(true);
	SunDirectionalLight->SetVisibility(false);
	bDay = false;
}

void ASDayNightSystem::OnDay()
{
	SkyAtmosphere->SetRayleighScattering(DayRaylayScattering);
	SkyAtmosphere->SetMultiScatteringFactor(1.f);
	SunDirectionalLight->SetVisibility(true);
	MoonDirectionalLight->SetVisibility(false);
	bDay = true;
}

void ASDayNightSystem::DayNightCycle(float DeltaTime)
{
	if (FMath::IsNearlyEqual(Time, EndDayTime, TimeMultiplier * DeltaTime) && bDay)
	{
		OnNightTriggeredDelegate.Broadcast();
		OnNight();
	}
	else if (FMath::IsNearlyEqual(Time, StartDayTime, TimeMultiplier * DeltaTime) && !bDay)
	{
		OnDayTriggeredDelegate.Broadcast();
		OnDay();
	}
	else if (Time > 24.f)
	{
		Day++;
		OnNewDayTriggeredDelegate.Broadcast();
		Time = 0.f;
	}
	SetSunMoonRotation();
}

void ASDayNightSystem::PickTimeMultiplier()
{
	switch (CycleSpeed)
	{
	case Custom: TimeMultiplier = 0.4f / TimeMultiplier; break;
	case x2Minutes: TimeMultiplier = 0.2f; break;
	case x3Minutes: TimeMultiplier = 0.13f; break;
	case x4Minutes: TimeMultiplier = 0.1f; break;
	case x5Minutes: TimeMultiplier = 0.08f; break;
	case x10Minutes: TimeMultiplier = 0.04f; break;
	default: break;
	}
}

void ASDayNightSystem::NewDayCallDelegate()
{
	Day++;
	if (Time < StartDayTime)
	{
		OnDayTriggeredDelegate.Broadcast();
		OnNightTriggeredDelegate.Broadcast();
	}
	else if (bDay)
	{
		OnNightTriggeredDelegate.Broadcast();
	}
}

void ASDayNightSystem::CurrentDayCallDelegate(float NewTime)
{
	if (NewTime > EndDayTime && Time < EndDayTime)
	{
		OnNightTriggeredDelegate.Broadcast();
		if (!bDay)
		{
			OnDayTriggeredDelegate.Broadcast();
		}
	}
	else if (NewTime > StartDayTime && !bDay)
	{
		OnDayTriggeredDelegate.Broadcast();
	}
	Time = NewTime;
}

void ASDayNightSystem::SetupSunHeightCurve()
{
	SunHeightCurve = NewObject<UCurveFloat>(this);
	float MidnightTime = EndDayTime + ((24.f - (EndDayTime - StartDayTime)) / 2.f);
	if (MidnightTime > 24.f)
	{
		float MidnightMultiplier = FMath::GetMappedRangeValueUnclamped(FVector2D(EndDayTime, MidnightTime), FVector2D(0.f, -3.f), 24.f);
		SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(24.f, MidnightMultiplier), ERichCurveInterpMode::RCIM_Cubic);
		SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(0.f, MidnightMultiplier), ERichCurveInterpMode::RCIM_Cubic);
		MidnightTime -= 24.f;
	}
	else
	{
		float MidnightMultiplier = FMath::GetMappedRangeValueUnclamped(FVector2D(MidnightTime, 24.f + StartDayTime), FVector2D(-3.f, 0.f), 24.f);
		SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(24.f, MidnightMultiplier), ERichCurveInterpMode::RCIM_Cubic);
		SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(0.f, MidnightMultiplier), ERichCurveInterpMode::RCIM_Cubic);
	}
	SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(MidnightTime, -3.f), ERichCurveInterpMode::RCIM_Cubic);
	SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(StartDayTime, -0.5f), ERichCurveInterpMode::RCIM_Cubic);
	SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(MiddayTime, 1.f), ERichCurveInterpMode::RCIM_Cubic);
	SunHeightCurve->FloatCurve.SetKeyInterpMode(SunHeightCurve->FloatCurve.UpdateOrAddKey(EndDayTime, -0.5f), ERichCurveInterpMode::RCIM_Cubic);
}

void ASDayNightSystem::SetupSkyMaterialParameters()
{
	UpdateSkyMaterial();
	MIDSky->SetScalarParameterValue(FName("CloudOpacity"), CloudOpacity);
	MIDSky->SetScalarParameterValue(FName("CloudSpeed"), CloudSpeed);
	MIDSky->SetScalarParameterValue(FName("StarsBrightness"), StarsBrightness);
}

void ASDayNightSystem::UpdateSkyMaterial()
{
	float SunHeight = SunHeightCurve->GetFloatValue(Time);
	MIDSky->SetVectorParameterValue(FName("HorizonColor"), HorizonColorCurve->GetClampedLinearColorValue(SunHeight));
	MIDSky->SetVectorParameterValue(FName("ZenithColor"), ZenithColorCurve->GetClampedLinearColorValue(SunHeight));
	MIDSky->SetVectorParameterValue(FName("CloudColor"), CloudColorCurve->GetClampedLinearColorValue(SunHeight));
	MIDSky->SetScalarParameterValue(FName("HorizonFalloff"), FMath::Lerp(3.f, 7.f, FMath::Abs(SunHeight)));
	MIDSky->SetScalarParameterValue(FName("SunHeight"), SunHeight);
}

void ASDayNightSystem::SetupSkyMaterial()
{
	SetupSunHeightCurve();
	if (CheckCurves())
	{
		if (SkyMaterial != nullptr) 
		{
			MIDSky = UMaterialInstanceDynamic::Create(SkyMaterial, this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Need material: M_Sky_Material"));
			bCurvesValid = false;
			return;
		}
		SkySphere->SetMaterial(0,MIDSky);
		if (MIDSky)
		{
			SetupSkyMaterialParameters();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Need all curves"));
	}
}

void ASDayNightSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Time += TimeMultiplier * DeltaTime;
	DayNightCycle(DeltaTime);
	if (bCurvesValid)
	{
		UpdateSkyMaterial();
	}
}

void ASDayNightSystem::SetNewTime(float NewTime)
{
	if (NewTime >= 0.f && NewTime <= 24.f)
	{
		if (NewTime < Time)
		{
			NewDayCallDelegate();
			Time = NewTime;
		}
		else
		{
			CurrentDayCallDelegate(NewTime);
		}
		(Time > StartDayTime && Time < EndDayTime) ? OnDay() : OnNight();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("New time must be between 0 and 24!"));
	}
}

void ASDayNightSystem::SetNewDay(int32 NewDay)
{
	if (NewDay >= 0)
	{
		Day = NewDay;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("The NewDay must be positive"));
	}
}