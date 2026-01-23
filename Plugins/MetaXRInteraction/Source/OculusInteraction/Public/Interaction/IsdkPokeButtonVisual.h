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

#include "IsdkIInteractableState.h"
#include "Components/SceneComponent.h"

#include "IsdkPokeButtonVisual.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class PokeButtonVisual;

namespace helper
{
class FPokeButtonVisualImpl;
}
} // namespace isdk::api

class UIsdkPokeInteractable;

/* SceneComponent representing the visual element of a button that responds to Poke Interactions,
 * must be paired with a PokeInteractable */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Poke Button Visual"))
class OCULUSINTERACTION_API UIsdkPokeButtonVisual : public USceneComponent
{
  GENERATED_BODY()

 public:
  UIsdkPokeButtonVisual();
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;

  bool IsApiInstanceValid() const;
  isdk::api::PokeButtonVisual* GetApiPokeButtonVisual();

  /* Returns the UIsdkPokeInteractable associated with this Visual */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkPokeInteractable* GetPokeInteractable() const
  {
    return PokeInteractable;
  }

  /* Sets the UIsdkPokeInteractable associated with this Visual */
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetPokeInteractable(UIsdkPokeInteractable* InPokeInteractable);
  virtual void OnUpdateTransform(
      EUpdateTransformFlags UpdateTransformFlags,
      ETeleportType Teleport = ETeleportType::None) override;

 private:
  float MaxExtentDistance;
  FTransform RelativeTransform;

  void SetWorldLocationFromInstance();
  void SubscribeInteractableEvents();
  void UnsubscribeInteractableEvents();
  void SetUpdatePositionEnabled(EIsdkInteractableState InteractableState);

  UFUNCTION()
  void HandleInteractableStateChanged(const FIsdkInteractableStateEvent& Event);

  UFUNCTION()
  void HandleWorldFrameFinishedEvent();

  UFUNCTION()
  void HandleWorldFrameStartedEvent();

  /* The UIsdkPokeInteractable associated with this Visual */
  UPROPERTY(
      BlueprintGetter = GetPokeInteractable,
      BlueprintSetter = SetPokeInteractable,
      Category = InteractionSDK)
  UIsdkPokeInteractable* PokeInteractable{};

  // Internal Impl
  TPimplPtr<isdk::api::helper::FPokeButtonVisualImpl> PokeButtonVisualImpl;
};
