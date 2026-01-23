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
#include "GameplayTagContainer.h"
#include "Rig/IsdkRigComponent.h"
#include "StructTypes.h"
#include "IsdkRigModifier.generated.h"

struct FIsdkInteractorStateEvent;

/*
   Abstract class for that is intended to trigger based on certain conditions (state events &
   matching gameplay tags currently supported); intended to support Rig Components.
*/
UCLASS(Blueprintable, Abstract, Category = InteractionSDK)
class OCULUSINTERACTIONPREBUILTS_API UIsdkRigModifier : public UObject
{
  GENERATED_BODY()

 public:
  UIsdkRigModifier(){};

  /* Called by the RigComponent during initialization (Implementable in Blueprint or C++) */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = InteractionSDK)
  void InitializeRigModifier(UIsdkRigComponent* CallingRigComponent);

  virtual void InitializeRigModifier_Implementation(UIsdkRigComponent* CallingRigComponent);

  /* Called by the RigComponent during EndPlay (Implementable in Blueprint or C++) */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = InteractionSDK)
  void ShutdownRigModifier(UIsdkRigComponent* CallingRigComponent);

  virtual void ShutdownRigModifier_Implementation(UIsdkRigComponent* CallingRigComponent)
      PURE_VIRTUAL(UIsdkRigModifier::ShutdownRigModifier);

  /* Any state entry in this array will cause this modifier to trigger */
  UPROPERTY(EditAnywhere, Category = InteractionSDK)
  TArray<EIsdkInteractorState> InteractorStateTriggers;

  /* Any state change entry in this array will cause this modifier to trigger */
  UPROPERTY(EditAnywhere, Category = InteractionSDK)
  TArray<FIsdkInteractorStateChangeArgs> InteractorStateChangeTriggers;

  /* Optional gameplay tags that must be on the interactor for this modifier to trigger */
  UPROPERTY(EditAnywhere, Category = InteractionSDK)
  FGameplayTagContainer RequiredInteractorGameplayTags;

  /* Optional gameplay tags that must be on the interactable for this modifier to trigger. Only
   * grabbable components currently supported. */
  UPROPERTY(EditAnywhere, Category = InteractionSDK)
  FGameplayTagContainer RequiredInteractableGameplayTags;

 protected:
  UPROPERTY()
  UIsdkRigComponent* ParentRigComponent = nullptr;

  bool ShouldModifierTrigger(FIsdkInteractorStateEvent StateEvent, UObject* InteractableReference);
};
