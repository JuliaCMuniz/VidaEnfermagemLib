/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "IsdkGrabbable.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "Math/UnrealMathUtility.h"

#include "IsdkGrabbableAudio.generated.h"

/// Raises events when an object is scaled up or down. Events are raised in steps,
/// meaning scale changes are only responded to when the scale magnitude delta since
/// last step exceeds a provided amount.
UCLASS(
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Grabbable Audio"))
class OCULUSINTERACTION_API UIsdkGrabbableAudio final : public UActorComponent
{
  GENERATED_BODY()

 public:
  UIsdkGrabbableAudio();
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FName TargetGrabbableName;

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Scaling",
      meta = (Tooltip = "The increase in scale magnitude that will fire the step event"))
  float StepSize = 0.6f;

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Scaling",
      meta = (Tooltip = "Events will not be fired more frequently than this many times per second"))
  int MaxEventFreq = 25;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Scaling")
  TArray<USoundBase*> ScalingSounds;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Grabbing")
  TArray<USoundBase*> GrabbingSounds;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Grabbing")
  TArray<USoundBase*> ReleasingSounds;

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Grabbing",
      meta =
          (Tooltip = "Sets the default pitch for the grabbing sound (1 is normal pitch)",
           ClampMin = ".1",
           ClampMax = "3.0",
           UIMin = ".1",
           UIMax = "3.0"))
  float GrabbingBasePitch{1};

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Grabbing",
      meta =
          (Tooltip =
               "Adjust the slider value for randomized pitch of the grabbing sound (0 is no randomization)",
           ClampMin = "-3",
           ClampMax = "3",
           UIMin = "-3",
           UIMax = "3"))
  float GrabbingPitchRange{.1f};

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Grabbing",
      meta =
          (Tooltip =
               "Sets the default volume for the grabbing sound (0 = silent, 1 = full volume)"))
  float GrabbingBaseVolume{1};

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Grabbing",
      meta =
          (Tooltip =
               "Adjust the slider value for randomized volume level playback if the grabbing sound (0 is no randomization)"))
  float GrabbingVolumeRange{0};

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Scaling",
      meta =
          (Tooltip = "Sets the default pitch for the scaling sound (1 is normal pitch)",
           ClampMin = ".1",
           ClampMax = "3.0",
           UIMin = ".1",
           UIMax = "3.0"))
  float ScalingBasePitch{1};

  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK|Scaling",
      meta =
          (Tooltip = "Sets the pitch range for the scaling sound ",
           ClampMin = "-3",
           ClampMax = "3",
           UIMin = "-3",
           UIMax = "3"))
  float ScalingPitchRange{.1};

  UFUNCTION(BlueprintSetter, Category = "InteractionSDK")
  void SetTargetGrabbable(UIsdkGrabbable* Grabbable);
  UFUNCTION(BlueprintGetter, Category = "InteractionSDK")
  UIsdkGrabbable* GetTargetGrabbable()
  {
    return TargetGrabbable;
  }
  UFUNCTION()
  void HandleGrabbableEvent(TransformEvent Event, const UIsdkGrabbable* Grabbable);

 private:
  UPROPERTY(
      BlueprintGetter = GetTargetGrabbable,
      BlueprintSetter = SetTargetGrabbable,
      Category = InteractionSDK)
  TObjectPtr<UIsdkGrabbable> TargetGrabbable;
  UPROPERTY()
  TObjectPtr<UAudioComponent> GrabbableAudioComponent = nullptr;

  FVector PreviousScale = FVector::OneVector;
  float LastEventTime = 0;
  int GrabPoseCount = 0;

  FVector GetCurrentScale();
  float GetScaleDelta();
  virtual void BeginPlay() override;
  void PreloadSounds(const TArray<USoundBase*>& Sounds);
  void PlaySound(
      const TArray<USoundBase*>& Sounds,
      float BasePitch,
      float HalfPitchRange,
      float BaseVolume,
      float HalfVolumeRange,
      bool bUsePitchRangeAsOffset = false,
      bool bUseVolumeRangeAsOffset = false);
  void UpdateScaling();
};
