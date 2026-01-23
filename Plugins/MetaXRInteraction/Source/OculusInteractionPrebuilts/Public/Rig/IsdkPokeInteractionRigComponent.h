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
#include "Components/ActorComponent.h"
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "IsdkPokeInteractionRigComponent.generated.h"

class UIsdkConditionalComponentIsActive;
class UIsdkConditionalGroupAll;
class UIsdkPokeInteractor;
class UIsdkPokeLimiterVisual;
class UIsdkSyntheticHand;

UCLASS(ClassGroup = (InteractionSDK))
class OCULUSINTERACTIONPREBUILTS_API UIsdkPokeInteractionRigComponent : public UActorComponent
{
  GENERATED_BODY()
 public:
  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkPokeInteractor> PokeInteractor;

  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkPokeLimiterVisual> PokeLimiterVisual;

  // Sets default values for this component's properties
  UIsdkPokeInteractionRigComponent();

  virtual void InitializeComponent() override;

  void BindDataSources(
      const FIsdkTrackingDataSources& DataSources,
      UIsdkSyntheticHand* SyntheticHand,
      USceneComponent* AttachToComponent,
      FName AttachToComponentSocket);

  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  UIsdkConditionalGroupAll* GetEnabledConditional() const
  {
    return EnabledConditionalGroup;
  }

 private:
  UFUNCTION()
  void HandleIsEnabledConditionalChanged(bool bIsEnabled);

  UPROPERTY(BlueprintGetter = GetEnabledConditional, Category = InteractionSDK)
  TObjectPtr<UIsdkConditionalGroupAll> EnabledConditionalGroup;

  UPROPERTY()
  TObjectPtr<UIsdkConditionalComponentIsActive> IsActiveConditional;
};
