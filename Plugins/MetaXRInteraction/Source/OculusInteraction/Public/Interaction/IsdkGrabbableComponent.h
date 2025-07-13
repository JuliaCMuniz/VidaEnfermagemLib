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
#include "Interaction/IsdkSceneInteractableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Pointable/IsdkIPointable.h"
#include "StructTypes.h"
#include "Grabbable/IsdkIGrabbable.h"
#include "IsdkGrabbableComponent.generated.h"

class UIsdkGrabberComponent;

UENUM()
enum class EIsdkGrabColliderType : uint8
{
  Unknown,
  Pinch,
  Palm,
  Custom
};

constexpr int GrabColliderTypeCount = static_cast<int>(EIsdkGrabColliderType::Custom);

/**
 * IsdkGrabbableComponent drives the ability for an actor to be grabbed, transformed, and thrown.
 * It is expected to be interacted with by a IsdkGrabberComponent on the player pawn.
 */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Grab Interactable"))
class OCULUSINTERACTION_API UIsdkGrabbableComponent : public UIsdkSceneInteractableComponent,
                                                      public IIsdkIPointable
{
  GENERATED_BODY()

 public:
  UIsdkGrabbableComponent();

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFn) override;

  virtual void GetInteractableStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractorState>>& OutInteractors) const override;

  // Event Getters
  virtual FIsdkInteractionPointerEventDelegate& GetInteractionPointerEventDelegate() override
  {
    return InteractionPointerEvent;
  }

  virtual void PostEvent(const FIsdkInteractionPointerEvent& Event);

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InteractionSDK")
  TSet<EIsdkGrabColliderType> GrabTypesAllowed{
      EIsdkGrabColliderType::Palm,
      EIsdkGrabColliderType::Pinch};

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  UPrimitiveComponent* GetCollider(const bool bForGrabbing = true);

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  void SetCollider(UPrimitiveComponent* Shape)
  {
    DefaultCollider = Shape;
    if (DefaultCollider)
    {
      DefaultCollider->SetGenerateOverlapEvents(true);
    }
  }
  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  void SetGrabCollider(UPrimitiveComponent* NewGrabCollider);

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  void SetPhysicsCollider(UPrimitiveComponent* NewPhysicsCollider);

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK|Interactable")
  bool IsSelectedBy(const UIsdkGrabberComponent* Grabber) const
  {
    if (SelectedGrabbers.Num() > 0 && Grabber != nullptr)
    {
      return SelectedGrabbers.Contains(Grabber);
    }
    return false;
  }

  bool IsHoveredBy(const UIsdkGrabberComponent* Grabber) const
  {
    if (HoveredGrabbers.Num() > 0 && Grabber != nullptr)
    {
      return HoveredGrabbers.Contains(Grabber);
    }
    return false;
  }

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  bool AllowsGrabType(EIsdkGrabColliderType Type)
  {
    return GrabTypesAllowed.Contains(Type);
  }

  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  void SetGrabTypeAllowed(EIsdkGrabColliderType Type, bool Allowed)
  {
    const bool CurrentlyAllowed = GrabTypesAllowed.Contains(Type);

    if (CurrentlyAllowed != Allowed)
    {
      if (!Allowed)
        GrabTypesAllowed.Remove(Type);
      else
        GrabTypesAllowed.Add(Type);
    }
  }

  // @brief Sends a cancel event to the IGrabbable that is being replaced with the identifier of all
  // @brief of the selected and hovered interactors
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetGrabbable(TScriptInterface<IIsdkIGrabbable> NewGrabbable);

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  TScriptInterface<IIsdkIGrabbable> GetGrabbable()
  {
    return Grabbable;
  }

 protected:
  bool IsHoveredBy(UIsdkGrabberComponent* Grabber)
  {
    return HoveredGrabbers.Contains(Grabber);
  }
  bool IsSelectedBy(UIsdkGrabberComponent* Grabber)
  {
    return SelectedGrabbers.Contains(Grabber);
  }
  virtual void SetState(EIsdkInteractableState NewState) override;

  UPROPERTY(Instanced)
  TObjectPtr<USceneComponent> ColliderComponent;

  UPROPERTY(EditAnywhere, Category = "InteractionSDK")
  FString GrabbableName;
  UPROPERTY(
      BlueprintSetter = SetGrabbable,
      BlueprintGetter = GetGrabbable,
      EditAnywhere,
      meta = (ExposeOnSpawn = true),
      Category = "InteractionSDK")
  TScriptInterface<IIsdkIGrabbable> Grabbable = {};

  UPROPERTY(Instanced)
  TObjectPtr<UPrimitiveComponent> DefaultCollider;

  UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Colliders")
  TObjectPtr<UPrimitiveComponent> GrabCollider;

  UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Colliders")
  TObjectPtr<UPrimitiveComponent> PhysicsCollider;

  UPROPERTY(
      EditAnywhere,
      meta = (UseComponentPicker, AllowedClasses = "PrimitiveComponent"),
      Category = "InteractionSDK|Colliders")
  FComponentReference GrabColliderReference;

  UPROPERTY(
      EditAnywhere,
      meta = (UseComponentPicker, AllowedClasses = "PrimitiveComponent"),
      Category = "InteractionSDK|Colliders")
  FComponentReference PhysicsColliderReference;

  UPrimitiveComponent* FindCollider();

  TArray<TObjectPtr<UIsdkGrabberComponent>> SelectedGrabbers;
  TArray<TObjectPtr<UIsdkGrabberComponent>> HoveredGrabbers;

  /* Delegate broadcasted when pointer events are triggered */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkInteractionPointerEventDelegate InteractionPointerEvent;

  UFUNCTION()
  void HandlePointerEvent(const FIsdkInteractionPointerEvent& PointerEvent);
};
