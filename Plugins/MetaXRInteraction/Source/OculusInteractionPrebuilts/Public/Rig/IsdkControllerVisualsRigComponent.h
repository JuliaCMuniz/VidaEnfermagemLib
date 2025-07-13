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
#include "IsdkRigComponent.h"
#include "IsdkTrackedDataSourceRigComponent.h"
#include "Templates/Function.h"
#include "IsdkControllerVisualsRigComponent.generated.h"

class UIsdkHandMeshComponent;
class UIsdkXRRigSettingsComponent;
class UIsdkInputActionsRigComponent;
class UIsdkControllerMeshComponent;
class UEnhancedInputComponent;
class UInputAction;
class UQuestControllerAnimInstance;
class UPoseableMeshComponent;
enum class EControllerHandBehavior : uint8;

/**
 * UIsdkControllerVisualsRigComponent holds logic pertaining to the visualization and tracking
 * of controllers (and any corresponding controller + hand visualization) while controllers are
 * the active tracked input (vs, say, hands).
 */
UCLASS(ClassGroup = (InteractionSDK))
class OCULUSINTERACTIONPREBUILTS_API UIsdkControllerVisualsRigComponent
    : public UIsdkTrackedDataSourceRigComponent
{
  GENERATED_BODY()

  friend class FIsdkControllerRigComponentTestBase;

 public:
  UIsdkControllerVisualsRigComponent();

  // Sets default values for our components (such as mesh and material assets, bone mappings).
  // These are only used for property defaults - they can be overridden by a user in the details
  // panel of the actor instance in the editor.
  void SetSubobjectPropertyDefaults(EIsdkHandType InHandType);

  virtual USceneComponent* GetTrackedVisual() const override;
  virtual USceneComponent* GetSyntheticVisual() const override
  {
    return nullptr;
  }
  virtual void GetInteractorSocket(
      USceneComponent*& OutSocketComponent,
      FName& OutSocketName,
      EIsdkHandBones HandBone) const override;
  void BindInputActions(
      UEnhancedInputComponent* EnhancedInputComponent,
      UIsdkInputActionsRigComponent* InputActionsRigComponent);

  USkeletalMeshComponent* GetAnimatedHandMeshComponent();
  UIsdkHandMeshComponent* GetPoseableHandMeshComponent();
  UIsdkControllerMeshComponent* GetControllerMeshComponent();

 protected:
  // A skeletal mesh used to represent the game controller
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkControllerMeshComponent> ControllerMeshComponent;

  // A skeletal mesh used to represent hands while holding a controller.  This mesh is driven
  // by an animations configured in-editor.
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<USkeletalMeshComponent> AnimatedHandMeshComponent;

  // A skeletal mesh used to represent hands while holding a controller.  This mesh is driven by
  // runtime-generated hand pose data.
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkHandMeshComponent> PoseableHandMeshComponent;

  virtual FTransform GetCurrentSyntheticTransform() override
  {
    return FTransform::Identity;
  }

  virtual void OnDataSourcesCreated() override;
  virtual void OnVisibilityUpdated(bool bTrackedVisibility, bool bSyntheticVisibility) override;

 private:
  // A helper to update a float value corresponding to an input action on any significant change in
  // state
  void BindAxisValue(
      UEnhancedInputComponent* EnhancedInputComponent,
      const UInputAction* Action,
      TFunction<void(UQuestControllerAnimInstance*, float)> Lambda) const;

  // A helper to update a boolean value corresponding to an input action on any significant change
  // in state
  void BindBoolValue(
      UEnhancedInputComponent* EnhancedInputComponent,
      const UInputAction* Action,
      TFunction<void(UQuestControllerAnimInstance*, bool)> Lambda) const;

  void UpdateAnimatedHandMeshVisibility(
      bool bTrackedVisibility,
      EControllerHandBehavior ControllerHandBehavior);
  void UpdatePoseableHandMeshVisibility(
      bool bTrackedVisibility,
      EControllerHandBehavior ControllerHandBehavior);
  void UpdateControllerMeshVisibility(
      bool bTrackedVisibility,
      EControllerHandBehavior ControllerHandBehavior);
};
