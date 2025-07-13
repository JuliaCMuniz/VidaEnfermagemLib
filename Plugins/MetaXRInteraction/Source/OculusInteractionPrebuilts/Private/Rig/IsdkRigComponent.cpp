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

#include "Rig/IsdkRigComponent.h"

#include "UObject/UObjectIterator.h"
#include "Subsystem/IsdkWidgetSubsystem.h"
#include "EnhancedInputComponent.h"
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "Interaction/IsdkRayInteractor.h"
#include "Rig/IsdkRigModifier.h"
#include "Engine/SkeletalMesh.h"
#include "CollisionShape.h"
#include "Interaction/IsdkPokeInteractor.h"
#include "Interaction/IsdkGrabberComponent.h"
#include "Rig/IsdkHandVisualsRigComponent.h"
#include "Materials/Material.h"
#include "DataSources/IsdkIHandJoints.h"
#include "DataSources/IsdkSyntheticHand.h"
#include "HandPoseDetection/IsdkHandFingerPinchGrabRecognizer.h"
#include "Kismet/GameplayStatics.h"
#include "Rig/IsdkControllerVisualsRigComponent.h"
#include "Rig/IsdkGrabInteractionRigComponent.h"
#include "Rig/IsdkInputActionsRigComponent.h"
#include "Rig/IsdkInteractionGroupRigComponent.h"
#include "Rig/IsdkPokeInteractionRigComponent.h"
#include "Rig/IsdkRayInteractionRigComponent.h"
#include "Utilities/IsdkXRUtils.h"

const FVector UIsdkRigComponent::ControllerPalmColliderOffset = FVector(-3.f, 0.f, -2.f);
const FVector UIsdkRigComponent::HandPalmColliderOffset = FVector(0.f, 3.f, -8.f);

// Sets default values
UIsdkRigComponent::UIsdkRigComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  RayInteractorSocket = EIsdkHandBones::HandWristRoot;
  PokeInteractorSocket = EIsdkHandBones::HandIndexTip;
  GrabberSocket = EIsdkHandBones::HandWristRoot;

  HandVisuals = CreateOptionalDefaultSubobject<UIsdkHandVisualsRigComponent>(TEXT("HandVisuals"));
  ControllerVisuals =
      CreateOptionalDefaultSubobject<UIsdkControllerVisualsRigComponent>(TEXT("ControllerVisuals"));
  InputActions = CreateDefaultSubobject<UIsdkInputActionsRigComponent>(TEXT("InputActions"));
  RayInteraction = CreateDefaultSubobject<UIsdkRayInteractionRigComponent>(TEXT("RayInteraction"));
  PokeInteraction =
      CreateDefaultSubobject<UIsdkPokeInteractionRigComponent>(TEXT("PokeInteraction"));
  GrabInteraction =
      CreateDefaultSubobject<UIsdkGrabInteractionRigComponent>(TEXT("GrabInteraction"));
  RayInteraction->PinchRecognizer = GrabInteraction->PinchGrabRecognizer;
  InteractionGroup =
      CreateDefaultSubobject<UIsdkInteractionGroupRigComponent>(TEXT("InteractionGroup"));
}

void UIsdkRigComponent::BeginPlay()
{
  Super::BeginPlay();

  // Subscribe to input action events
  if (bAutoBindInputActions)
  {
    const auto FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

    AActor* OwnerActor = GetOwner();
    OwnerActor->EnableInput(FirstLocalPlayer);
    const auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwnerActor->InputComponent);
    if (ensureMsgf(
            IsValid(EnhancedInputComponent),
            TEXT("IsdkRigHand: Could not create UEnhancedInputComponent.")))
    {
      BindInputActions(EnhancedInputComponent);
    }
  }

  // Register interactors to the widget system
  RegisterInteractorWidgetIndices();

  // Setup interaction groups
  CreateInteractionGroupConditionals();

  // Spawn & Initialize Rig Modifiers
  for (TSubclassOf<UIsdkRigModifier> RigModifierClass : RigModifiersToSpawn)
  {
    UIsdkRigModifier* NewRigModifier = NewObject<UIsdkRigModifier>(this, RigModifierClass);
    NewRigModifier->InitializeRigModifier(this);
    ActiveRigModifiers.Add(NewRigModifier);
  }
}

void UIsdkRigComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);

  UnregisterInteractorWidgetIndices();

  // Shut Down Active Rig Modifiers
  for (UIsdkRigModifier* ThisRigModifier : ActiveRigModifiers)
  {
    ThisRigModifier->ShutdownRigModifier(this);
  }
}

void UIsdkRigComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!bAreDataSourcesReady)
  {
    if (!IsValid(HmdDataSource.GetObject()))
    {
      InitializeHmdDataSource();
    }
    if (IsValid(HandVisuals) && !IsValid(HandVisuals->AttachedToMotionController))
    {
      if (HandVisuals->TryAttachToParentMotionController(this))
      {
        OnHandVisualsAttached();
      }
    }
    if (IsValid(ControllerVisuals) && !IsValid(ControllerVisuals->AttachedToMotionController))
    {
      if (ControllerVisuals->TryAttachToParentMotionController(this))
      {
        OnControllerVisualsAttached();
      }
    }

    bAreDataSourcesReady = IsValid(HmdDataSource.GetObject()) &&
        IsValid(GetVisuals()->GetDataSources().DataSourceComponent);
    if (bAreDataSourcesReady)
    {
      // Tell interested parties that data sources are available.
      DataSourcesReady.Broadcast();
    }
  }
}

UIsdkPokeInteractor* UIsdkRigComponent::GetPokeInteractor() const
{
  return PokeInteraction->PokeInteractor;
}

UIsdkRayInteractor* UIsdkRigComponent::GetRayInteractor() const
{
  return RayInteraction->RayInteractor;
}

UIsdkPokeLimiterVisual* UIsdkRigComponent::GetPokeLimiterVisual() const
{
  return PokeInteraction->PokeLimiterVisual;
}

UIsdkGrabberComponent* UIsdkRigComponent::GetGrabber() const
{
  return GrabInteraction->Grabber;
}

UIsdkHandVisualsRigComponent* UIsdkRigComponent::GetHandVisuals() const
{
  return HandVisuals;
}

UIsdkControllerVisualsRigComponent* UIsdkRigComponent::GetControllerVisuals() const
{
  return ControllerVisuals;
}

UIsdkRayInteractionRigComponent* UIsdkRigComponent::GetRayInteraction() const
{
  return RayInteraction;
}

UIsdkPokeInteractionRigComponent* UIsdkRigComponent::GetPokeInteraction() const
{
  return PokeInteraction;
}

UIsdkGrabInteractionRigComponent* UIsdkRigComponent::GetGrabInteraction() const
{
  return GrabInteraction;
}

UIsdkInputActionsRigComponent* UIsdkRigComponent::GetInputActions() const
{
  return InputActions;
}

UIsdkInteractionGroupRigComponent* UIsdkRigComponent::GetInteractionGroup() const
{
  return InteractionGroup;
}

const FIsdkVirtualUserInfo& UIsdkRigComponent::GetWidgetVirtualUser() const
{
  return WidgetVirtualUser;
}

void UIsdkRigComponent::SetWidgetVirtualUser(const FIsdkVirtualUserInfo& InWidgetVirtualUser)
{
  WidgetVirtualUser = InWidgetVirtualUser;
}

const TArray<UIsdkRigModifier*>& UIsdkRigComponent::GetActiveRigModifiers() const
{
  return ActiveRigModifiers;
}

bool UIsdkRigComponent::GetHmdDataSource(
    TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceOut) const
{
  HmdDataSourceOut = HmdDataSource;
  return IsValid(HmdDataSource.GetObject());
}

void UIsdkRigComponent::SetRigComponentDefaults(EIsdkHandType HandType)
{
  check(HandType != EIsdkHandType::None);

  if (IsValid(HandVisuals))
  {
    HandVisuals->SetSubobjectPropertyDefaults(HandType);
  }
  if (IsValid(ControllerVisuals))
  {
    ControllerVisuals->SetSubobjectPropertyDefaults(HandType);
  }
  InputActions->SetSubobjectPropertyDefaults(HandType);

  if (HandType == EIsdkHandType::HandLeft)
  {
    WidgetVirtualUser.VirtualUserIndex = 0;
  }
  else if (HandType == EIsdkHandType::HandRight)
  {
    WidgetVirtualUser.VirtualUserIndex = 1;
  }
}

UIsdkTrackedDataSourceRigComponent* UIsdkRigComponent::GetVisuals() const
{
  UIsdkTrackedDataSourceRigComponent* Visuals = IsValid(HandVisuals)
      ? static_cast<UIsdkTrackedDataSourceRigComponent*>(HandVisuals)
      : static_cast<UIsdkTrackedDataSourceRigComponent*>(ControllerVisuals);

  checkf(
      IsValid(Visuals),
      TEXT("%s: Either HandVisuals or ControllerVisuals must be set"),
      *GetFullName()) return Visuals;
}

void UIsdkRigComponent::UpdateComponentDataSources()
{
  UIsdkTrackedDataSourceRigComponent* Visuals = GetVisuals();
  UIsdkSyntheticHand* SyntheticHand = IsValid(HandVisuals) ? HandVisuals->SyntheticHand : nullptr;
  const FIsdkTrackingDataSources& DataSources = Visuals->GetDataSources();

  // UpdateComponentDataSources may be called before data sources have been initialized.
  if (!DataSources.bIsInitialized)
  {
    return;
  }

  FName InteractorAttachSocket = NAME_None;
  USceneComponent* InteractorAttachComponent = nullptr;

  // Wire up data sources to interactors that work with either controllers or hands
  Visuals->GetInteractorSocket(
      InteractorAttachComponent, InteractorAttachSocket, RayInteractorSocket);
  RayInteraction->BindDataSources(
      DataSources, HmdDataSource, InteractorAttachComponent, InteractorAttachSocket);

  Visuals->GetInteractorSocket(
      InteractorAttachComponent, InteractorAttachSocket, PokeInteractorSocket);
  PokeInteraction->BindDataSources(
      DataSources, SyntheticHand, InteractorAttachComponent, InteractorAttachSocket);

  Visuals->GetInteractorSocket(InteractorAttachComponent, InteractorAttachSocket, GrabberSocket);
  GrabInteraction->BindDataSources(
      DataSources, HmdDataSource, InteractorAttachComponent, InteractorAttachSocket);

  GrabInteraction->UpdateMeshDependencies(
      GetPalmColliderOffset(), GetPinchAttachMesh(), GetThumbTipSocketName());

  // Some things only work with a hand.
  if (IsValid(HandVisuals))
  {
    // Bind the optional 'Hand Pinch' gesture to the grab interactor, since we have a HandVisual.
    GrabInteraction->BindHandGrabInput(HandVisuals->TrackedHandVisual, EIsdkFingerType::Index);

    // Setting Inbound HandData Reference to our DataSource from HandVisual
    if (IsValid(HandVisuals->SyntheticHand))
    {
      DataSources.DataSourceComponent->SetInboundHandData(
          IIsdkIHandJoints::Execute_GetHandData(HandVisuals->SyntheticHand));
    }
  }
}

void UIsdkRigComponent::RegisterInteractorWidgetIndices()
{
  if (GetOwner()->HasActorBegunPlay() || GetOwner()->IsActorBeginningPlay())
  {
    // Registering the interactors with the ISDK Widget Subsystem will allow them to send
    // VirtualUserPointerEvents to Slate Widgets.
    UIsdkWidgetSubsystem& IsdkWidgetSubsystem = UIsdkWidgetSubsystem::Get(GetWorld());
    IsdkWidgetSubsystem.RegisterVirtualUserInfo(GetPokeInteractor(), WidgetVirtualUser);
    IsdkWidgetSubsystem.RegisterVirtualUserInfo(GetRayInteractor(), WidgetVirtualUser);
  }
}

void UIsdkRigComponent::UnregisterInteractorWidgetIndices()
{
  UIsdkWidgetSubsystem& IsdkWidgetSubsystem = UIsdkWidgetSubsystem::Get(GetWorld());
  IsdkWidgetSubsystem.UnregisterVirtualUserInfo(GetPokeInteractor());
  IsdkWidgetSubsystem.UnregisterVirtualUserInfo(GetRayInteractor());
}

FVector UIsdkRigComponent::GetPalmColliderOffset() const
{
  return FVector::ZeroVector;
}

USkinnedMeshComponent* UIsdkRigComponent::GetPinchAttachMesh() const
{
  return nullptr;
}

void UIsdkRigComponent::CreateInteractionGroupConditionals()
{
  constexpr auto PokeInteractorGroupBehavior = FIsdkInteractionGroupMemberBehavior{
      .bDisableOnOtherSelect = true, .bDisableOnOtherNearFieldHover = false, .bIsNearField = true};
  auto CalculateInteractorGroupMemberState = [](const FIsdkInteractorStateEvent& Event)
  {
    FIsdkInteractionGroupMemberState State{};
    if (IsValid(Event.Interactor.GetObject()))
    {
      State.bIsSelectStateBlocking = Event.Interactor->HasSelectedInteractable();
    }
    return State;
  };

  PokeInteraction->GetEnabledConditional()->AddConditional(InteractionGroup->AddInteractor(
      PokeInteraction->PokeInteractor,
      *PokeInteraction->PokeInteractor->GetInteractorStateChangedDelegate(),
      CalculateInteractorGroupMemberState,
      PokeInteractorGroupBehavior));

  constexpr auto RayInteractorGroupBehavior = FIsdkInteractionGroupMemberBehavior{
      .bDisableOnOtherSelect = true,
      .bDisableOnOtherNearFieldHover = true,
      .bIsNearField = false // false here means a ray wont prevent near-field interactors hovering.
  };
  RayInteraction->GetEnabledConditional()->AddConditional(InteractionGroup->AddInteractor(
      RayInteraction->RayInteractor,
      *RayInteraction->RayInteractor->GetInteractorStateChangedDelegate(),
      CalculateInteractorGroupMemberState,
      RayInteractorGroupBehavior));

  constexpr auto GrabInteractorGroupBehavior = FIsdkInteractionGroupMemberBehavior{
      .bDisableOnOtherSelect = true,
      .bDisableOnOtherNearFieldHover = true, // disable grab when poke is hovering
      .bIsNearField = true};
  GrabInteraction->GetEnabledConditional()->AddConditional(InteractionGroup->AddInteractor(
      GrabInteraction->Grabber,
      *GrabInteraction->Grabber->GetInteractorStateChangedDelegate(),
      CalculateInteractorGroupMemberState,
      GrabInteractorGroupBehavior));
}

void UIsdkRigComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
  // This just ensures we don't call this twice
  // Override this method to do the actual input assignments
  check(!bHasBoundInputActions);
  bHasBoundInputActions = true;

  RayInteraction->BindInputActions(
      EnhancedInputComponent, InputActions->SelectAction, InputActions->SelectStrengthAction);
  GrabInteraction->BindControllerGrabInput(
      EnhancedInputComponent, InputActions->PinchGrabAction, InputActions->PalmGrabAction);
  if (ControllerVisuals)
  {
    ControllerVisuals->BindInputActions(EnhancedInputComponent, InputActions);
  }

  // Pinch select input mapping is currently not available for OpenXR hands so we use the native
  // finger recognizer events to trigger pinch select/unselect
  if (IsdkXRUtils::IsUsingOpenXR())
  {
    RayInteraction->PinchRecognizer->PinchGrabStarted.AddUniqueDynamic(
        RayInteraction->RayInteractor, &UIsdkRayInteractor::Select);
    RayInteraction->PinchRecognizer->PinchGrabFinished.AddUniqueDynamic(
        RayInteraction->RayInteractor, &UIsdkRayInteractor::Unselect);
  }
}

void UIsdkRigComponent::InitializeHmdDataSource()
{
  // If our HMD Data Source is good, we're done
  if (IsValid(HmdDataSource.GetObject()))
  {
    return;
  }

  AActor* OwningActor = this->GetAttachParentActor();
  if (!ensureMsgf(
          IsValid(OwningActor),
          TEXT("UIsdkRigComponent::InitializeHmdDataSource() - OwningActor isn't valid!")))
  {
    return;
  }

  TArray<UIsdkRigComponent*> ActorRigComponents;
  OwningActor->GetComponents(UIsdkRigComponent::StaticClass(), ActorRigComponents);

  // Check every other RigComponent on this actor and if any of them already have an HMD Data
  // Source, grab a reference to that
  for (UIsdkRigComponent* RigComponent : ActorRigComponents)
  {
    if (RigComponent == this || !IsValid(RigComponent))
    {
      continue;
    }
    if (RigComponent->GetHmdDataSource(HmdDataSource))
    {
      break;
    }
  }

  // Our search was in vain, set up an HMD Data Source ourselves
  if (!IsValid(HmdDataSource.GetObject()))
  {
    const auto TrackingDataSubsystem = GetVisuals()->GetTrackingDataSubsystem();
    if (ensureMsgf(
            IsValid(TrackingDataSubsystem.GetObject()),
            TEXT(
                "UIsdkRigComponent::InitializeHmdDataSource() - Could not get the IsdkTrackingDataSubsystem.")))
    {
      HmdDataSource = IIsdkITrackingDataSubsystem::Execute_CreateHmdDataSourceComponent(
          TrackingDataSubsystem.GetObject(), GetOwner());
    }
  }
}

FName UIsdkRigComponent::GetThumbTipSocketName() const
{
  // Pull the thumb tip socket name through one of the available avenues.
  // The socket is always named the same, and so we should ultimately reorganize this in a way
  // in which we don't need to pull it from a locally-defined member variable, but this will do
  // for now.
  FName ThumbTipSocketName = NAME_None;
  if (HandVisuals)
  {
    ThumbTipSocketName = HandVisuals->TrackedHandVisual
                             ->MappedBoneNames[static_cast<int>(EIsdkHandBones::HandThumbTip)];
  }
  else if (ControllerVisuals)
  {
    ThumbTipSocketName = ControllerVisuals->GetPoseableHandMeshComponent()
                             ->MappedBoneNames[static_cast<int>(EIsdkHandBones::HandThumbTip)];
  }

  return ThumbTipSocketName;
}
