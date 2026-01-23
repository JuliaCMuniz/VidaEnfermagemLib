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
#include "IsdkTrackedDataSourceRigComponent.h"
#include "Components/ActorComponent.h"

#include "IsdkHandVisualsRigComponent.generated.h"

class UIsdkHandMeshComponent;
class UIsdkSyntheticHand;
class UIsdkOneEuroFilterDataModifier;
class USceneComponent;
class UMaterial;
class USkeletalMesh;

/**
 * UIsdkHandVisualsRigComponent holds logic pertaining to the visualization of hands and tracking
 * of hands while hands are the active tracked input (vs, say, controllers).
 */
UCLASS(ClassGroup = (InteractionSDK))
class OCULUSINTERACTIONPREBUILTS_API UIsdkHandVisualsRigComponent
    : public UIsdkTrackedDataSourceRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkHandVisualsRigComponent();

  virtual void InitializeComponent() override;

  // must be called from Target's constructor.
  void SetSubobjectPropertyDefaults(EIsdkHandType InHandType);

  virtual USceneComponent* GetTrackedVisual() const override;
  virtual USceneComponent* GetSyntheticVisual() const override;

  virtual void GetInteractorSocket(
      USceneComponent*& OutSocketComponent,
      FName& OutSocketName,
      EIsdkHandBones HandBone) const override;

  // A synthetic hand, which follows the tracked hand, but for which the positioning and posing may
  // be altered by interaction
  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkSyntheticHand> SyntheticHand;

  // Visuals corresponding directly to the user's hand
  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkHandMeshComponent> TrackedHandVisual;

  // Visuals corresponding to the user's hand, for which the positioning and posing may be altered
  // by interaction
  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkHandMeshComponent> SyntheticHandVisual;

  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkOneEuroFilterDataModifier> OneEuroFilterDataModifier;

 protected:
  virtual void OnDataSourcesCreated() override;

 private:
  virtual FTransform GetCurrentSyntheticTransform() override;

  void SetHandVisualPropertyDefaults(
      UIsdkHandMeshComponent* HandMesh,
      USkeletalMesh* SkeletalMesh,
      UMaterial* Material);
};
