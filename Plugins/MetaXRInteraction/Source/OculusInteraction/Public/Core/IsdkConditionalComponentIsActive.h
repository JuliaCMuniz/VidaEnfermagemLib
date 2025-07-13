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
#include "IsdkConditional.h"
#include "IsdkConditionalComponentIsActive.generated.h"

/**
 * Watches an ActorComponents IsActive() flag, setting the state of this conditional to the value
 * of that flag.
 */
UCLASS(Blueprintable, DefaultToInstanced, Category = InteractionSDK)
class OCULUSINTERACTION_API UIsdkConditionalComponentIsActive : public UIsdkConditional
{
  GENERATED_BODY()

 public:
  UIsdkConditionalComponentIsActive();

  /* Returns the Actor Component the active state of which this conditional is watching */
  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  UActorComponent* GetComponent() const
  {
    return ActorComponent;
  }

  /* Sets the Actor Component the active state of which this conditional should watch */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetComponent(UActorComponent* InActorComponent);

 private:
  UPROPERTY(
      BlueprintGetter = GetComponent,
      BlueprintSetter = SetComponent,
      Category = InteractionSDK)
  UActorComponent* ActorComponent{};

  UFUNCTION()
  void HandleComponentActivated(UActorComponent* Component, bool bReset)
  {
    CalculateResolvedValue();
  }

  UFUNCTION()
  void HandleComponentDeactivated(UActorComponent* Component)
  {
    CalculateResolvedValue();
  }

  void CalculateResolvedValue();
};
