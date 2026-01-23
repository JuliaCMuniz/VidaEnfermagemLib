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
#include "IsdkHandData.h"
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "Components/ActorComponent.h"
#include "Core/IsdkConditionalGroupAll.h"
#include "IsdkTrackedDataSourceRigComponent.generated.h"

UENUM()
enum class EIsdkRigHandVisibility : uint8
{
  // Do not automatically adjust visibility of the hand visuals, even if the device is disabled.
  // The synthetic hand visual will by default be visible, and tracked hand mesh invisible.
  Manual,

  // Hides all visuals when tracking source is disconnected. Only show Tracked hand, when connected.
  TrackedOnly,

  // Hides all visuals when tracking source is disconnected. Only show Synthetic hand, when
  // connected.
  SyntheticOnly,

  // When the synthetic hand has different data than the tracked hand, make that visible. Otherwise,
  // make the tracked hand mesh visible.
  SyntheticWhenRootChanged,
};

UCLASS(ClassGroup = (InteractionSDK), Abstract)
class OCULUSINTERACTIONPREBUILTS_API UIsdkTrackedDataSourceRigComponent : public UActorComponent
{
  GENERATED_BODY()

 public:
  // Sets default values for this component's properties
  UIsdkTrackedDataSourceRigComponent();

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  const FIsdkTrackingDataSources& GetDataSources() const;

  bool TryAttachToParentMotionController(USceneComponent* Target);
  void CreateDataSourcesAsTrackedHand();
  void CreateDataSourcesAsTrackedController();

  void UpdateComponentVisibility();

  virtual void GetInteractorSocket(
      USceneComponent*& OutSocketComponent,
      FName& OutSocketName,
      EIsdkHandBones HandBone = EIsdkHandBones::Invalid) const
  {
  }

  /**
   * @brief Optionally update the visibility of the hand visuals each frame based on the
   * state of the synthetic hand.
   */
  UPROPERTY(
      BlueprintReadWrite,
      EditAnywhere,
      meta = (ExposeOnSpawn = true),
      Category = InteractionSDK)
  EIsdkRigHandVisibility HandVisibilityMode = EIsdkRigHandVisibility::SyntheticOnly;

  /**
   * @brief Which hand this rig represents. This field is used to determine the handedness of the
   * default hand meshes.
   */
  UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = InteractionSDK)
  EIsdkHandType HandType;

  /**
   * @brief Optional: Use this given subsystem instead of the default one that exists on the world.
   * The subsystem is used to create components that are able to read hand tracking data.
   */
  UPROPERTY()
  TScriptInterface<IIsdkITrackingDataSubsystem> CustomTrackingData;

  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UMotionControllerComponent> AttachedToMotionController;

  virtual USceneComponent* GetTrackedVisual() const
      PURE_VIRTUAL(UIsdkTrackedDataSourceRigComponent::GetTrackedVisual, return nullptr;);
  virtual USceneComponent* GetSyntheticVisual() const
      PURE_VIRTUAL(UIsdkTrackedDataSourceRigComponent::GetSyntheticVisual, return nullptr;);

  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  UIsdkConditionalGroupAll* GetForceOffVisibility() const;

  TScriptInterface<IIsdkITrackingDataSubsystem> GetTrackingDataSubsystem();

 protected:
  virtual FTransform GetCurrentSyntheticTransform()
  {
    return {};
  }

  virtual void OnDataSourcesCreated() {}
  virtual void OnVisibilityUpdated(bool bTrackedVisibility, bool bSyntheticVisibility) {}

  UFUNCTION()
  void HandleIsdkFrameFinished();

  FIsdkTrackingDataSources LocalDataSources;

  UPROPERTY()
  TObjectPtr<UIsdkConditionalGroupAll> ForceOffVisibility;
};
