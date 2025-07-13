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
#include "EnhancedInputComponent.h"
#include "Core/IsdkConditionalComponentIsActive.h"
#include "Core/IsdkConditionalGroupAll.h"
#include "HandPoseDetection/IsdkHandFingerPinchGrabRecognizer.h"
#include "InteractorVisuals/IsdkRayVisual.h"
#include "IsdkRayInteractionRigComponent.generated.h"

class UIsdkRayInteractor;
class UInputAction;
class UIsdkHandMeshComponent;
struct FIsdkTrackingDataSources;

/**
 *
 */
UCLASS(ClassGroup = (InteractionSDK))
class OCULUSINTERACTIONPREBUILTS_API UIsdkRayInteractionRigComponent : public UActorComponent
{
  GENERATED_BODY()

 public:
  UIsdkRayInteractionRigComponent();
  virtual void InitializeComponent() override;

  UFUNCTION()
  void BindDataSources(
      const FIsdkTrackingDataSources& DataSources,
      TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceIn,
      USceneComponent* AttachToComponent,
      FName AttachToComponentSocket);

  UFUNCTION()
  void UpdateRayVisual();

  /**
   * @brief Binds input actions used by the interactors to the given component. This method will not
   * undo any previous bindings, therefore should only be called once. If you intend to call this,
   * make sure to set `bAutoBindInputActions` to false.
   */
  void BindInputActions(
      UEnhancedInputComponent* EnhancedInputComponent,
      UInputAction* InRaySelectAction,
      UInputAction* InRaySelectStrengthAction);

  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkHandFingerPinchGrabRecognizer> PinchRecognizer;

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  float GetRaySelectStrength();

  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkRayInteractor> RayInteractor;

  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  UIsdkConditionalGroupAll* GetEnabledConditional() const
  {
    return EnabledConditionalGroup;
  }

  // Properties for configuration
  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  TObjectPtr<UIsdkRayVisualComponent> RayVisual;

  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InteractionSDK|Ray Customization")
  TSubclassOf<UIsdkRayVisualComponent> RayVisualClass;

  UPROPERTY()
  TScriptInterface<IIsdkIHmdDataSource> HmdDataSource;

 private:
  void CreateRayVisual(AActor* AttachParent);

  void HandleIsEnabledConditionalChanged(bool bIsEnabled);

  UPROPERTY(BlueprintGetter = GetEnabledConditional, Category = InteractionSDK)
  TObjectPtr<UIsdkConditionalGroupAll> EnabledConditionalGroup;

  UPROPERTY()
  TObjectPtr<UIsdkConditionalComponentIsActive> IsActiveConditional;

  FEnhancedInputActionValueBinding* RaySelectStrengthIAValue = nullptr;
};
