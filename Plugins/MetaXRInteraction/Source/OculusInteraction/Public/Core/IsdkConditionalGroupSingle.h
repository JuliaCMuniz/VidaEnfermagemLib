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
#include "IsdkConditionalGroup.h"
#include "IsdkConditionalGroupSingle.generated.h"

/**
 * Conditional Group that resolves true if ONE and only one conditional is true
 */
UCLASS(Blueprintable, DefaultToInstanced, Category = InteractionSDK)
class OCULUSINTERACTION_API UIsdkConditionalGroupSingle : public UIsdkConditionalGroup
{
  GENERATED_BODY()
 public:
  UIsdkConditionalGroupSingle()
  {
    SetResolvedValue(CalculateValueFromEmpty());
  };

  bool CalculateValueFromConditionals() const override
  {
    int CountTrue = 0;
    for (const auto& Conditional : ConditionalDelegateHandles)
    {
      if (Conditional.Key->GetResolvedValue())
      {
        ++CountTrue;
      }
    }
    return CountTrue == 1;
  }

  bool CalculateValueFromEmpty() const override
  {
    return false;
  }
};
