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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataSources/IsdkIHmdDataSource.h"
#include "StructTypes.h"
#include "InputAction.h"
#include "IsdkHandData.h"
#include "IsdkRayVisual.h"
#include "IsdkRayVisualFunctionLibrary.generated.h"

class UIsdkRayVisualComponent;
class UIsdkRayInteractor;

/**
 *
 */
UCLASS(BlueprintType)
class OCULUSINTERACTIONPREBUILTS_API UIsdkRayVisualFunctionLibrary final
    : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

 public:
  /// <summary>
  ///
  /// </summary>
  /// <param name="CursorPosition"></param>
  /// <param name="ReferenceDistance">Distance at which the scale of the cursor is 1.0</param>
  /// <param name="HmdDataSource"></param>
  /// <returns></returns>
  static float GetCursorScaleFromDistanceToHmd(
      const FVector CursorPosition,
      const float ReferenceDistance,
      const TScriptInterface<IIsdkIHmdDataSource>& HmdDataSource);
  static FTransform GetCursorTransformFromCollision(FIsdkOptionalSurfaceHit CollisionInfo);
  static UInputAction* GetPinchStrengthIA(EIsdkHandType HandType);
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  static void UpdateRayVisualWithRayInteractor(
      UIsdkRayVisualComponent* RayVisual,
      UIsdkRayInteractor* RayInteractor,
      float RaySelectStrength);
};
