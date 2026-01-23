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
#include "StructTypes.h"
#include "../Pointable/IsdkInteractionPointerEvent.h"
#include "IsdkITransformer.h"
#include "IsdkIGrabbable.h"
#include "../IsdkThrowable.h"
#include "IsdkGrabbable.generated.h"

UENUM(Blueprintable)
enum class TransformEvent : uint8
{
  BeginTransform,
  UpdateTransform,
  EndTransform
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FIsdkGrabbableEventDelegate,
    TransformEvent,
    Event,
    const UIsdkGrabbable*,
    Grabbable);

UCLASS(ClassGroup = (InteractionSDK), meta = (BlueprintSpawnableComponent))
class OCULUSINTERACTION_API UIsdkGrabbable : public UActorComponent, public IIsdkIGrabbable
{
  GENERATED_BODY()

 public:
  // Sets default values for this component's properties
  UIsdkGrabbable();
  virtual FIsdkCancelGrabEventDelegate* GetCancelGrabEventDelegate() override
  {
    return &CancelGrabEvent;
  }
  virtual void ProcessPointerEvent(const FIsdkInteractionPointerEvent& Event) override;

  void RestartTransformer()
  {
    EndTransform();
    BeginTransform();
  }

  // @brief Sends a cancel event to all the grab event sources
  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  void ForceCancel();

  // @brief This function also calls the IIsdkITransformer::UpdateConstraints function with the
  // @brief current "TargetInitialTransform" value.
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetSingleGrabTransformer(TScriptInterface<IIsdkITransformer> Transformer)
  {
    SingleGrabTransformer = Transformer;
    if (IsValid(SingleGrabTransformer.GetObject()))
    {
      SingleGrabTransformer->UpdateConstraints(TargetInitialTransform);
    }
    RestartTransformer();
  }
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  TScriptInterface<IIsdkITransformer> GetSingleGrabTransformer()
  {
    return SingleGrabTransformer;
  }

  // @brief This function also calls the IIsdkITransformer::UpdateConstraints function with the
  // @brief current "TargetInitialTransform" value.
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetMultiGrabTransformer(TScriptInterface<IIsdkITransformer> Transformer)
  {
    MultiGrabTransformer = Transformer;
    if (IsValid(MultiGrabTransformer.GetObject()))
    {
      MultiGrabTransformer->UpdateConstraints(TargetInitialTransform);
    }
    RestartTransformer();
  }
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  TScriptInterface<IIsdkITransformer> GetMultiGrabTransformer()
  {
    return MultiGrabTransformer;
  }

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetTransformTarget(USceneComponent* NewTarget)
  {
    TransformTarget = NewTarget;
    TargetInitialTransform = FIsdkTargetTransform(TransformTarget);
    UpdateTransformerConstraints();
  }
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  USceneComponent* GetTransformTarget()
  {
    return TransformTarget;
  }
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const FIsdkGrabPoseCollection& GetPoseCollection() const
  {
    return GrabPoses;
  }
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  const TScriptInterface<IIsdkITransformer>& GetActiveGrabTransformer() const
  {
    return ActiveGrabTransformer;
  }

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  const void UpdateTransformerConstraints()
  {
    if (IsValid(SingleGrabTransformer.GetObject()))
    {
      SingleGrabTransformer->UpdateConstraints(TargetInitialTransform);
    }
    if (IsValid(MultiGrabTransformer.GetObject()))
    {
      MultiGrabTransformer->UpdateConstraints(TargetInitialTransform);
    }
  }
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InteractionSDK")
  FName TransformTargetName;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InteractionSDK")
  EIsdkGrabType GrabType = EIsdkGrabType::MultiGrab;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InteractionSDK|Throwable")
  bool bIsThrowable = true;
  UPROPERTY(
      BlueprintReadWrite,
      EditAnywhere,
      meta = (EditCondition = "bIsThrowable == true", EditConditionHides),
      Category = "InteractionSDK|Throwable")
  bool EnableGravityWhenThrown = true;

  FIsdkGrabbableEventDelegate* GetGrabbableEventDelegate()
  {
    return &GrabbableEvent;
  }

 protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
  {
    EndTransform();
    Super::EndPlay(EndPlayReason);
  }
  void BeginTransform();
  void UpdateTransform();
  void EndTransform();
  FIsdkTargetTransform TargetInitialTransform;

 private:
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkGrabbableEventDelegate GrabbableEvent;
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkCancelGrabEventDelegate CancelGrabEvent;
  UPROPERTY(
      BlueprintSetter = SetSingleGrabTransformer,
      BlueprintGetter = GetSingleGrabTransformer,
      EditAnywhere,
      meta = (ExposeOnSpawn = true),
      Category = "InteractionSDK")
  TScriptInterface<IIsdkITransformer> SingleGrabTransformer;
  UPROPERTY(
      BlueprintSetter = SetMultiGrabTransformer,
      BlueprintGetter = GetMultiGrabTransformer,
      EditAnywhere,
      meta =
          (ExposeOnSpawn = true,
           EditCondition = "GrabType == EIsdkGrabType::MultiGrab",
           EditConditionHides),
      Category = "InteractionSDK")
  TScriptInterface<IIsdkITransformer> MultiGrabTransformer;
  UPROPERTY(
      BlueprintSetter = SetTransformTarget,
      BlueprintGetter = GetTransformTarget,
      meta = (ExposeOnSpawn = true),
      Category = "InteractionSDK")
  TObjectPtr<USceneComponent> TransformTarget;
  UPROPERTY(BlueprintGetter = GetPoseCollection, Category = "InteractionSDK")
  FIsdkGrabPoseCollection GrabPoses;
  TScriptInterface<IIsdkITransformer> ActiveGrabTransformer = nullptr;

  UPROPERTY(EditAnywhere, Category = "InteractionSDK|Throwable")
  FIsdkThrowableSettings ThrowSettings{};
  UPROPERTY(EditAnywhere, Category = "InteractionSDK|Throwable")
  TObjectPtr<UIsdkThrowable> ThrowableComponent;
  bool bWasSimulatingPhysics = false;
  bool bWasPhysicsCached = false;
};
