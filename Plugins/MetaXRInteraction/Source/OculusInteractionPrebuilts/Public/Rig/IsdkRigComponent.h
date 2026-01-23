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
#include "Subsystem/IsdkWidgetSubsystem.h"
#include "IsdkRigComponent.generated.h"

class UIsdkPokeLimiterVisual;
class UIsdkPokeInteractor;
class UIsdkGrabberComponent;
class UIsdkHandFingerPinchGrabRecognizer;
class UIsdkRigModifier;
class UIsdkTrackedDataSourceRigComponent;
class UEnhancedInputComponent;
class IIsdkIHmdDataSource;
enum class EIsdkHandBones : uint8;
enum class EIsdkHandType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIsdkRigComponentLifecycleEvent);

/**
 * A prebuilt collection of interactors for a single hand.
 *
 * The IsdkLegacyRigHand itself is an abstract class; there are derived types for Left and Right
 * hands that can be instantiated directly. The derived types set appropriate default values for
 * bones and meshes.
 *
 * To customize bone mappings, hand meshes or the behavior of the interactors, either:
 * - If placing an instance of an IsdkLegacyRigHand derived actor in the level: change the desired
 *   component properties directly in the details panel of the actor.
 * - If spawning an IsdkLegacyRigHand derived actor via a Child Actor Component in an actor: change
 * the desired component properties in the "Child Actor Template / InteractionSDK" sub-section.
 */

UCLASS(
    Abstract,
    Blueprintable,
    ClassGroup = ("InteractionSDK|Rig"),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Rig Component"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkRigComponent : public USceneComponent
{
  GENERATED_BODY()

 public:
  // Default constructor required for Unreal object system.
  UIsdkRigComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  void CreateInteractionGroupConditionals();

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkPokeInteractor* GetPokeInteractor() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkRayInteractor* GetRayInteractor() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkPokeLimiterVisual* GetPokeLimiterVisual() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkGrabberComponent* GetGrabber() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkHandVisualsRigComponent* GetHandVisuals() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkControllerVisualsRigComponent* GetControllerVisuals() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkRayInteractionRigComponent* GetRayInteraction() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkPokeInteractionRigComponent* GetPokeInteraction() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkGrabInteractionRigComponent* GetGrabInteraction() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkInputActionsRigComponent* GetInputActions() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkInteractionGroupRigComponent* GetInteractionGroup() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const FIsdkVirtualUserInfo& GetWidgetVirtualUser() const;

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetWidgetVirtualUser(const FIsdkVirtualUserInfo& InWidgetVirtualUser);

  /* Returns all of the active rig modifiers successfully spawned from RigModifiersToSpawn*/
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const TArray<UIsdkRigModifier*>& GetActiveRigModifiers() const;

  /* Returns true if HMD DataSource is valid and returns reference to it */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool GetHmdDataSource(TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceOut) const;

  /**
   * @brief When true, during BeginPlay this actor will bind the configured input actions to the
   * PlayerController at index 0.
   * If false, a manual call to BindInputActionEvents must be made to bind the input actions.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  bool bAutoBindInputActions = true;

  /**
   * @brief Binds input actions used by the interactors to the given component. This method will not
   * undo any previous bindings, therefore should only be called once. If you intend to call this,
   * make sure to set `bAutoBindInputActions` to false.
   */
  virtual void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);

  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InteractionSDK|Customization")
  EIsdkHandBones RayInteractorSocket;

  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InteractionSDK|Customization")
  EIsdkHandBones PokeInteractorSocket;

  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InteractionSDK|Customization")
  EIsdkHandBones GrabberSocket;

  /* All Rig Modifiers that this Rig Component should spawn and initialize */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  TArray<TSubclassOf<UIsdkRigModifier>> RigModifiersToSpawn;

  UIsdkTrackedDataSourceRigComponent* GetVisuals() const;

  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkRigComponentLifecycleEvent DataSourcesReady;

 protected:
  virtual void OnHandVisualsAttached() {}
  virtual void OnControllerVisualsAttached() {}
  virtual void UpdateComponentDataSources();
  virtual void RegisterInteractorWidgetIndices();
  virtual void UnregisterInteractorWidgetIndices();
  virtual FVector GetPalmColliderOffset() const;
  virtual USkinnedMeshComponent* GetPinchAttachMesh() const;

  void SetRigComponentDefaults(EIsdkHandType HandType);

  // Attempts to find another already created HmdDataSource on other RigComponents. If unable,
  // creates one. Sets either result to member variable.
  void InitializeHmdDataSource();

  bool bHasBoundInputActions = false;
  bool bAreDataSourcesReady = false;
  static const FVector ControllerPalmColliderOffset;
  static const FVector HandPalmColliderOffset;

  // Properties for access of attached components.
  UPROPERTY(BlueprintGetter = GetHandVisuals, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkHandVisualsRigComponent> HandVisuals;

  UPROPERTY(BlueprintGetter = GetControllerVisuals, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkControllerVisualsRigComponent> ControllerVisuals;

  UPROPERTY(BlueprintGetter = GetRayInteraction, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkRayInteractionRigComponent> RayInteraction;

  UPROPERTY(BlueprintGetter = GetPokeInteraction, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkPokeInteractionRigComponent> PokeInteraction;

  UPROPERTY(BlueprintGetter = GetGrabInteraction, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkGrabInteractionRigComponent> GrabInteraction;

  UPROPERTY(BlueprintGetter = GetInputActions, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkInputActionsRigComponent> InputActions;

  UPROPERTY(BlueprintGetter = GetInteractionGroup, EditAnywhere, Category = InteractionSDK)
  TObjectPtr<UIsdkInteractionGroupRigComponent> InteractionGroup;

  UPROPERTY(
      BlueprintGetter = GetWidgetVirtualUser,
      BlueprintSetter = SetWidgetVirtualUser,
      EditAnywhere,
      Category = InteractionSDK,
      meta = (ExposeOnSpawn = true))
  FIsdkVirtualUserInfo WidgetVirtualUser;

  UPROPERTY()
  TArray<UIsdkRigModifier*> ActiveRigModifiers;

  UPROPERTY()
  TScriptInterface<IIsdkIHmdDataSource> HmdDataSource;

 private:
  FName GetThumbTipSocketName() const;
};
