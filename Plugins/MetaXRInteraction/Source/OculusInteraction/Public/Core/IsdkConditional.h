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
#include "UObject/Object.h"
#include "IsdkConditional.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FIsdkResolvedValueChangedDelegate, bool);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FIsdkResolvedValueChangedDynamicDelegate,
    bool,
    NewValue);

/**
 * A conditional contains a single boolean value, with an event that is fired whenever the value
 * changes.
 */
UCLASS(Blueprintable, Abstract, Category = InteractionSDK)
class OCULUSINTERACTION_API UIsdkConditional : public UObject
{
  GENERATED_BODY()
 public:
  /* Retrieve the currently resolved boolean value of this conditional */
  UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = InteractionSDK)
  bool GetResolvedValue() const
  {
    return bResolvedValue;
  }

 protected:
  void SetResolvedValue(bool bInValue)
  {
    if (bInValue != bResolvedValue)
    {
      bResolvedValue = bInValue;
      if (ResolvedValueChanged.IsBound())
      {
        ResolvedValueChanged.Broadcast(bInValue);
      }
      if (ResolvedValueChangedBp.IsBound())
      {
        ResolvedValueChangedBp.Broadcast(bInValue);
      }
    }
  }

 private:
  UPROPERTY(BlueprintGetter = GetResolvedValue, EditAnywhere, Category = InteractionSDK)
  bool bResolvedValue{};

 public:
  // Non-Dynamic event for fast c++ event listeners
  FIsdkResolvedValueChangedDelegate ResolvedValueChanged;

  // Dynamic event for slower blueprint event listeners
  UPROPERTY(BlueprintReadWrite, Category = InteractionSDK, DisplayName = ResolvedValueChanged)
  FIsdkResolvedValueChangedDynamicDelegate ResolvedValueChangedBp;
};
