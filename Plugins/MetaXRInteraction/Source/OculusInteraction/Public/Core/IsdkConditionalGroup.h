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
#include "IsdkConditional.h"
#include "UObject/Object.h"
#include "Delegates/IDelegateInstance.h"
#include "IsdkConditionalGroup.generated.h"

/**
 * Collection of IsdkConditionals that are resolved as a group (abstract base class)
 */
UCLASS(Abstract, NotBlueprintable, Category = InteractionSDK)
class OCULUSINTERACTION_API UIsdkConditionalGroup : public UIsdkConditional
{
  GENERATED_BODY()
 public:
  /* Returns the conditionals collectively resolved in this group */
  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  void GetConditionals(TArray<UIsdkConditional*>& Conditionals) const
  {
    ConditionalDelegateHandles.GetKeys(Conditionals);
  }

  /* Removes a conditional from being resolved in this group */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void RemoveConditional(UIsdkConditional* InConditional)
  {
    auto Handle = ConditionalDelegateHandles.FindAndRemoveChecked(InConditional);
    if (Handle != FDelegateHandle{})
    {
      check(InConditional != nullptr);
      InConditional->ResolvedValueChanged.Remove(Handle);

      ReCalculateResolvedValue();
    }
  }
  /* Add a conditional to be resolved in this group */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void AddConditional(UIsdkConditional* InConditional)
  {
    if (ensure(InConditional) && !ConditionalDelegateHandles.Contains(InConditional))
    {
      auto Handle = InConditional->ResolvedValueChanged.AddWeakLambda(
          this, [this](bool) { ReCalculateResolvedValue(); });
      ConditionalDelegateHandles.Add(InConditional, Handle);

      ReCalculateResolvedValue();
    }
  }

  /* Returns true if there are no conditionals being resolved in this group */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool IsEmpty()
  {
    return ConditionalDelegateHandles.Num() == 0;
  }

 protected:
  UIsdkConditionalGroup(){};

  virtual bool CalculateValueFromConditionals() const
      PURE_VIRTUAL(UIsdkConditionalGroup::CalculateValueFromConditionals, return false;);

  virtual bool CalculateValueFromEmpty() const
      PURE_VIRTUAL(UIsdkConditionalGroup::CalculateValueFromEmpty, return false;);

  void ReCalculateResolvedValue();
  TMap<UIsdkConditional*, FDelegateHandle> ConditionalDelegateHandles;
};
