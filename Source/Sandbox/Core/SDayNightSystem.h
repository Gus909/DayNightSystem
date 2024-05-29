// Illia Huskov 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDayNightSystem.generated.h"

UENUM(BlueprintType)
enum ECycleSpeed : uint8
{
	x2Minutes UMETA(DisplayName = "2 Minutes"),
	x3Minutes UMETA(DisplayName = "3 Minutes"),
	x4Minutes UMETA(DisplayName = "4 Minutes"),
	x5Minutes UMETA(DisplayName = "5 Minutes"),
	x10Minutes UMETA(DisplayName = "10 Minutes"),
	Custom UMETA(DisplayName = "Custom"),
};

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;
class UTimelineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimePeriodChangeSignature);

UCLASS()
class SANDBOX_API ASDayNightSystem : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnTimePeriodChangeSignature OnDayTriggeredDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTimePeriodChangeSignature OnNightTriggeredDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTimePeriodChangeSignature OnNewDayTriggeredDelegate;

private:

	TObjectPtr<USceneComponent> ActorRootComponent;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDirectionalLightComponent> SunDirectionalLight;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> SkySphere;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent> SunMoonAxis;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDirectionalLightComponent> MoonDirectionalLight;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDSky;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInstance> SkyMaterial;
#pragma region Curves

	bool bCurvesValid;
	UPROPERTY()
	TObjectPtr<UCurveFloat> SunHeightCurve;

	UPROPERTY(EditDefaultsOnly, Category = "DNS|Curves")
	TObjectPtr<UCurveLinearColor> HorizonColorCurve;

	UPROPERTY(EditDefaultsOnly, Category = "DNS|Curves")
	TObjectPtr<UCurveLinearColor> CloudColorCurve;

	UPROPERTY(EditDefaultsOnly, Category = "DNS|Curves")
	TObjectPtr<UCurveLinearColor> ZenithColorCurve;

#pragma endregion Curves
#pragma region SkyParameters
	UPROPERTY(EditAnywhere, Category = "DNS|Sky Settings")
	float CloudSpeed = 0.1f;
	UPROPERTY(EditAnywhere, Category = "DNS|Sky Settings")
	float CloudOpacity = 0.7f;
	UPROPERTY(EditAnywhere, Category = "DNS|Sky Settings")
	float StarsBrightness = 0.1f;
#pragma endregion SkyParameters
#pragma region TimeParameters

	bool bDay = true;

	float MiddayTime = 12.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 24.f), Category = "DNS|Time Settings")
	float Time = 12.f;
	/** Speed of the in-game day*/
	UPROPERTY(EditAnywhere, Category = "DNS|Time Settings")
	TEnumAsByte<ECycleSpeed> CycleSpeed = ECycleSpeed::x5Minutes;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f), Category = "DNS|Time Settings")
	int32 Day = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 5.f, ClampMax = 7.f), Category = "DNS|Time Settings")
	float StartDayTime = 6.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 16.f, ClampMax = 22.f), Category = "DNS|Time Settings")
	float EndDayTime = 18.f;

	/** In minute, 0 equal static */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DaySpeed==ECycleSpeed::Custom", EditConditionHides, ClampMin = 0.f, ClampMax = 60.f),
		Category = "DNS|Time Settings")
	float TimeMultiplier = 0.4f;
#pragma endregion TimeParameters
#pragma region DayParameters

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 10.f), Category = "DNS|Day Settings")
	float SunLightIntensity = 5.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 5.f), Category = "DNS|Day Settings")
	float SunLightSourceAngle = 5.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 359.f), Category = "DNS|Day Settings")
	float SunLightYawRotation = 5.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 16000.f), Category = "DNS|Day Settings")
	float SunLightTemperature = 6000.f;

	UPROPERTY(EditAnywhere, Category = "DNS|Day Settings")
	FColor SunLightColor = FColor(255, 255, 255);

	UPROPERTY(EditAnywhere, Category = "DNS|Day Settings")
	FColor DayRaylayScattering = FColor(99, 129, 204);

#pragma endregion DayParameters
#pragma region NightParameters
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 10.f), Category = "DNS|Night Settings")
	float MoonLightIntensity = 1.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 0.2f), Category = "DNS|Night Settings")
	float MoonLightBrightness = 0.1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "DNS|Night Settings")
	float MoonScale = 0.1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 2.f), Category = "DNS|Night Settings")
	float MoonRotation = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 16000.f), Category = "DNS|Night Settings")
	float MoonLightTemperature = 9000.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "DNS|Night Settings")
	float NighttimeMultiScattering = 0.5f;

	UPROPERTY(EditAnywhere, Category = "DNS|Night Settings")
	FColor MoonColor = FColor(255, 255, 255);

	UPROPERTY(EditAnywhere, Category = "DNS|Night Settings")
	FColor MoonLightColor = FColor(77, 96, 119);

	UPROPERTY(EditAnywhere, Category = "DNS|Night Settings")
	FColor NighttimeRaylayScattering = FColor(63, 76, 161);

#pragma endregion NightParameters

public:
	ASDayNightSystem();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetNewTime(float NewTime);

	UFUNCTION(BlueprintCallable)
	void SetNewDay(int32 NewDay);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetTime() const { return Time; };

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void SetSunMoonRotation();
	void SetupNightParameters();
	void SetupDayParameters();
	void OnNight();
	void OnDay();
	void DayNightCycle(float DeltaTime);
	void PickTimeMultiplier();
	void NewDayCallDelegate();
	void CurrentDayCallDelegate(float NewTime);
	void SetupSunHeightCurve();
	void SetupSkyMaterialParameters();
	void UpdateSkyMaterial();
	void SetupSkyMaterial();
	FORCEINLINE bool CheckCurves() { return bCurvesValid = SunHeightCurve != nullptr && HorizonColorCurve != nullptr && CloudColorCurve != nullptr && ZenithColorCurve != nullptr; };
};