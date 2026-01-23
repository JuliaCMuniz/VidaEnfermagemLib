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
#include "IsdkGrabbableComponent.h"
#include "Containers/Map.h"
#include "Interaction/IsdkSceneInteractorComponent.h"
#include "StructTypes.h"
#include "IsdkGrabberComponent.generated.h"

class UIsdkGrabbableComponent;
class USphereComponent;

USTRUCT()
struct FIsdkColliderInfo
{
  GENERATED_BODY()

  EIsdkInteractorState State{EIsdkInteractorState::Normal};
  UPROPERTY(Instanced)
  TMap<UPrimitiveComponent*, UIsdkGrabbableComponent*> HoverObjects{};
  UPROPERTY(Instanced)
  UIsdkGrabbableComponent* SelectObject{};

  // RankIndex is a way to figure which collider has most recently started hovering over a grabbable
  int RankIndex{0};
};

/**
 * IsdkGrabberComponent drives the ability for a pawn to interact with actors that have an
 * IsdkGrabbableComponent attached to them.  This component uses a number of configurable colliders
 * per grab type (eg, pinch / palm grab) to drive detection of grabbables.  Selection/Unselection
 * is expected to be driven externally by an IsdkGrabInteractionRigComponent.
 */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Grabber Component"))
class OCULUSINTERACTION_API UIsdkGrabberComponent : public UIsdkSceneInteractorComponent
{
  GENERATED_BODY()

 public:
  UIsdkGrabberComponent();

  /* Returns all PrimitiveComponents used by this Interactor of the given IsdkGrabColliderType */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void GetCollidersByType(EIsdkGrabColliderType Type, TArray<UPrimitiveComponent*>& outArray)
  {
    outArray.Reset();
    for (auto& PrimitiveToColliderInfo : CollidersByType[Type])
    {
      outArray.Add(PrimitiveToColliderInfo.Key);
    }
  }

  /* Returns true if any colliders of the given IsdkGrabColliderType exist on this Interactor */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool HasCollidersOfType(EIsdkGrabColliderType Type)
  {
    return !CollidersByType[Type].IsEmpty();
  }

  bool GetColliderInfo(
      UPrimitiveComponent* Collider,
      EIsdkGrabColliderType Type,
      FIsdkColliderInfo& OutColliderInfo)
  {
    if (CollidersByType.Contains(Type))
    {
      auto& CollidersOfType = CollidersByType[Type];
      if (const FIsdkColliderInfo* ColliderInfo = CollidersOfType.Find(Collider))
      {
        OutColliderInfo = *ColliderInfo;
        return true;
      }
    }
    return false;
  }

  void SetColliderInfo(
      UPrimitiveComponent* Collider,
      EIsdkGrabColliderType Type,
      const FIsdkColliderInfo& ColliderInfo)
  {
    if (CollidersByType.Contains(Type))
    {
      auto& CollidersOfType = CollidersByType[Type];
      CollidersOfType.Emplace(Collider, ColliderInfo);
    }
  }
  /* Adds a PrimitiveComponent of the given IsdkGrabColliderType to this Interactor */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void AddCollider(UPrimitiveComponent* Collider, EIsdkGrabColliderType Type);

  /* Removes a PrimitiveComponent of the given IsdkGrabColliderType from this Interactor */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void RemoveCollider(UPrimitiveComponent* Collider, EIsdkGrabColliderType Type)
  {
    auto& CollidersOfType = CollidersByType[Type];
    if (CollidersOfType.Contains(Collider))
    {
      CollidersOfType.Remove(Collider);
    }
  }

  UFUNCTION()
  void ComputeBestGrabbableColliderForGrabType(
      EIsdkGrabColliderType Type,
      UPrimitiveComponent*& MyCollider,
      UPrimitiveComponent*& GrabbableCollider,
      UIsdkGrabbableComponent*& Grabbable);

  virtual void BeginPlay() override;
  void UnregisterAllColliders();
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;
  virtual void Deactivate() override;

  virtual bool HasCandidate() const override;
  virtual bool HasInteractable() const override;
  virtual bool HasSelectedInteractable() const override;

  virtual void UpdateState();
  void UpdatePalmOffset(const FVector& InPalmOffset);

  virtual FIsdkInteractionRelationshipCounts GetInteractorStateRelationshipCounts() const override;

  virtual void GetInteractorStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractableState>>& OutInteractables) const override;

  UFUNCTION()
  void SelectPinch();
  UFUNCTION()
  void SelectPalm();
  UFUNCTION()
  void UnselectPinch();
  UFUNCTION()
  void UnselectPalm();

  UFUNCTION()
  void HandleGrabbableCancelEvent(int Identifier, TScriptInterface<IIsdkIGrabbable> GrabbableSender)
  {
    if (Identifier == ID && bIsGrabbing)
    {
      UnselectByGrabType(SelectingColliderType);
    }
  }

  void SelectByGrabType(EIsdkGrabColliderType Type);
  void UnselectByGrabType(EIsdkGrabColliderType Type);

  /* Returns true if the grabber is grabbing and the component it is grabbing*/
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool GetGrabbingStateWithComponent(UIsdkGrabbableComponent*& GrabbableOut) const;

  /* Whether or not this Grabber will activate when a palm grab is recognized */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  bool bAllowPalmGrab = true;

  /* Whether or not this Grabber will activate when a pinch grab grab is recognized */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  bool bAllowPinchGrab = true;

  /* UPrimitiveComponent to use as a collider for interactable selection */
  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UPrimitiveComponent> SelectingCollider;

  UPROPERTY()
  bool bIsGrabbing = false;

  /* Radius of the to set when initializing for the pinch grab overlap collider */
  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = InteractionSDK)
  float PinchColliderRadius = 1.2f;

  /* Radius of the to set when initializing for the palm grab overlap collider */
  UPROPERTY(
      BlueprintReadOnly,
      EditAnywhere,
      meta = (EditCondition = "bAllowPalmGrab == true", EditConditionHides),
      Category = InteractionSDK)
  float PalmColliderRadius = 4.0f;

 private:
  UFUNCTION(BlueprintInternalUseOnly, Category = InteractionSDK)
  void BeginOverlap(
      UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult& SweepResult);

  UFUNCTION(BlueprintInternalUseOnly, Category = InteractionSDK)
  void EndOverlap(
      UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex);

  void UpdateOverlappedSet();

  void PostEvent(EIsdkPointerEventType Type, UIsdkGrabbableComponent* Dest);
  virtual void TickComponent(
      float DeltaTime,
      enum ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;
  void UnregisterCollider(UPrimitiveComponent* Collider, EIsdkGrabColliderType Type);

  EIsdkGrabColliderType FindColliderType(UPrimitiveComponent* Collider) const;

  int64 PointerEventToken = 0;

  UPROPERTY(Instanced)
  TObjectPtr<UIsdkGrabbableComponent> CandidateGrabbable;

  UPROPERTY(Instanced)
  TObjectPtr<UIsdkGrabbableComponent> GrabbedComponent;

  UPROPERTY()
  TObjectPtr<USphereComponent> DefaultPalmCollider;

  using PrimitiveComponentToColliderInfoMap = TMap<UPrimitiveComponent*, FIsdkColliderInfo>;
  using ColliderTypeToInfoMap = TMap<EIsdkGrabColliderType, PrimitiveComponentToColliderInfoMap>;
  ColliderTypeToInfoMap CollidersByType;

  UPROPERTY()
  TSet<UIsdkGrabbableComponent*> OverlappedGrabbables;

  int NextRankIndex = 0;
  EIsdkGrabColliderType SelectingColliderType = EIsdkGrabColliderType::Unknown;

  void DrawDebugVisuals() const;
  FVector GetSelectingColliderPosition();
};
