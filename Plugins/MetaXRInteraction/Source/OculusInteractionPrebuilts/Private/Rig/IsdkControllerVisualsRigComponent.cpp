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

#include "Rig/IsdkControllerVisualsRigComponent.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "IsdkContentAssetPaths.h"
#include "IsdkControllerMeshComponent.h"
#include "IsdkFunctionLibrary.h"
#include "IsdkHandMeshComponent.h"
#include "Animation/QuestControllerAnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/Material.h"
#include "Rig/IsdkInputActionsRigComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Utilities/IsdkXRUtils.h"

UIsdkControllerVisualsRigComponent::UIsdkControllerVisualsRigComponent()
{
  ControllerMeshComponent =
      CreateDefaultSubobject<UIsdkControllerMeshComponent>(TEXT("ControllerMesh"));
  AnimatedHandMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMesh"));

  PoseableHandMeshComponent =
      CreateDefaultSubobject<UIsdkHandMeshComponent>(TEXT("PoseableHandMesh"));
  PoseableHandMeshComponent->SetMappedBoneNamesAsDefault(HandType);
  PoseableHandMeshComponent->SetRootPoseIgnored(true);

  // Adjust the collision settings of the ControllerMesh component
  ControllerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  ControllerMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void UIsdkControllerVisualsRigComponent::SetSubobjectPropertyDefaults(EIsdkHandType InHandType)
{
  HandType = InHandType;

  struct FStaticObjectFinders
  {
    // Controllers
    ConstructorHelpers::FObjectFinder<USkeletalMesh> LeftControllerMeshFinder;
    ConstructorHelpers::FObjectFinder<USkeletalMesh> RightControllerMeshFinder;
    ConstructorHelpers::FClassFinder<UAnimInstance> LeftControllerAnimBlueprintFinder;
    ConstructorHelpers::FClassFinder<UAnimInstance> RightControllerAnimBlueprintFinder;

    // Hands
    ConstructorHelpers::FObjectFinder<USkeletalMesh> LeftHandMeshFinder;
    ConstructorHelpers::FObjectFinder<USkeletalMesh> RightHandMeshFinder;
    ConstructorHelpers::FClassFinder<UAnimInstance> LeftControllerHandAnimBlueprintFinder;
    ConstructorHelpers::FClassFinder<UAnimInstance> RightControllerHandAnimBlueprintFinder;

    FStaticObjectFinders()
        : LeftControllerMeshFinder(
              IsdkContentAssetPaths::Models::Controller::OculusControllerSkeletalMeshL),
          RightControllerMeshFinder(
              IsdkContentAssetPaths::Models::Controller::OculusControllerSkeletalMeshR),
          LeftControllerAnimBlueprintFinder(
              IsdkContentAssetPaths::Models::Controller::OculusControllerAnimBlueprintL),
          RightControllerAnimBlueprintFinder(
              IsdkContentAssetPaths::Models::Controller::OculusControllerAnimBlueprintR),
          LeftHandMeshFinder(IsdkContentAssetPaths::Models::Hand::OpenXRLeftHand),
          RightHandMeshFinder(IsdkContentAssetPaths::Models::Hand::OpenXRRightHand),
          LeftControllerHandAnimBlueprintFinder(
              IsdkContentAssetPaths::ControllerDrivenHands::ControllerDrivenHandAnimBlueprintL),
          RightControllerHandAnimBlueprintFinder(
              IsdkContentAssetPaths::ControllerDrivenHands::ControllerDrivenHandAnimBlueprintR)
    {
    }
  };

  static FStaticObjectFinders StaticObjectFinders;
  USkeletalMesh* DefaultControllerMesh = nullptr;
  TSubclassOf<UAnimInstance> DefaultControllerAnimInstanceClass;

  USkeletalMesh* DefaultHandMesh = nullptr;
  TSubclassOf<UAnimInstance> DefaultControllerHandAnimInstanceClass;

  if (InHandType == EIsdkHandType::HandLeft)
  {
    DefaultControllerMesh = StaticObjectFinders.LeftControllerMeshFinder.Object;
    DefaultControllerAnimInstanceClass =
        StaticObjectFinders.LeftControllerAnimBlueprintFinder.Class;
    DefaultHandMesh = StaticObjectFinders.LeftHandMeshFinder.Object;
    DefaultControllerHandAnimInstanceClass =
        StaticObjectFinders.LeftControllerHandAnimBlueprintFinder.Class;
  }
  else if (InHandType == EIsdkHandType::HandRight)
  {
    DefaultControllerMesh = StaticObjectFinders.RightControllerMeshFinder.Object;
    DefaultControllerAnimInstanceClass =
        StaticObjectFinders.RightControllerAnimBlueprintFinder.Class;
    DefaultHandMesh = StaticObjectFinders.RightHandMeshFinder.Object;
    DefaultControllerHandAnimInstanceClass =
        StaticObjectFinders.RightControllerHandAnimBlueprintFinder.Class;
  }

  // Set mesh and material properties to default values.
  // These can be overridden by a user in the details panel of the actor instance in the editor.
  if (ensureMsgf(
          IsValid(DefaultControllerMesh),
          TEXT("%s: Failed to find controller mesh"),
          *GetFullNameSafe(ControllerMeshComponent->GetOwner())))
  {
    ControllerMeshComponent->SetSkeletalMesh(DefaultControllerMesh);
    ControllerMeshComponent->SetAnimationMode(EAnimationMode::Type::AnimationBlueprint);
    ControllerMeshComponent->SetAnimInstanceClass(DefaultControllerAnimInstanceClass);
  }

  if (IsValid(AnimatedHandMeshComponent))
  {
    if (ensureMsgf(
            IsValid(DefaultHandMesh),
            TEXT("%s: Failed to find hand mesh"),
            *GetFullNameSafe(AnimatedHandMeshComponent->GetOwner())))
    {
      AnimatedHandMeshComponent->SetSkeletalMesh(DefaultHandMesh);
      AnimatedHandMeshComponent->SetAnimationMode(EAnimationMode::Type::AnimationBlueprint);
      AnimatedHandMeshComponent->SetAnimInstanceClass(DefaultControllerHandAnimInstanceClass);
    }
  }

  if (IsValid(PoseableHandMeshComponent))
  {
    if (ensureMsgf(
            IsValid(DefaultHandMesh),
            TEXT("%s: Failed to find hand mesh"),
            *GetFullNameSafe(AnimatedHandMeshComponent->GetOwner())))
    {
      PoseableHandMeshComponent->SetSkinnedAsset(DefaultHandMesh);
    }
  }

  // Controllers do not have a synthetic visual.
  HandVisibilityMode = EIsdkRigHandVisibility::TrackedOnly;
}

USceneComponent* UIsdkControllerVisualsRigComponent::GetTrackedVisual() const
{
  return ControllerMeshComponent;
}

void UIsdkControllerVisualsRigComponent::GetInteractorSocket(
    USceneComponent*& OutSocketComponent,
    FName& OutSocketName,
    EIsdkHandBones HandBone) const
{
  const auto ControllerHandBehavior = UIsdkFunctionLibrary::GetControllerHandBehavior(GetWorld());
  const bool bUseHandAttachment =
      ControllerHandBehavior == EControllerHandBehavior::HandsOnlyAnimated ||
      ControllerHandBehavior == EControllerHandBehavior::HandsOnlyProcedural;

  OutSocketName = NAME_None;
  if (bUseHandAttachment)
  {
    // Attach to the animated hand if in animated mode, attach to poseable hand if in procedural
    // mode.
    if (ControllerHandBehavior == EControllerHandBehavior::HandsOnlyAnimated)
    {
      OutSocketComponent = AnimatedHandMeshComponent;
    }
    else
    {
      OutSocketComponent = PoseableHandMeshComponent;
    }

    // Get the bone name for the corresponding input HandBone.  Note that both hands use the same
    // OpenXR hand skeleton, so the mapping is the same either way.
    if (HandBone != EIsdkHandBones::Invalid)
    {
      OutSocketName = PoseableHandMeshComponent->MappedBoneNames[static_cast<size_t>(HandBone)];
    }
  }
  else
  {
    OutSocketComponent = ControllerMeshComponent->GetPointerPoseComponent();
  }
}

void UIsdkControllerVisualsRigComponent::BindInputActions(
    UEnhancedInputComponent* EnhancedInputComponent,
    UIsdkInputActionsRigComponent* InputActionsRigComponent)
{
  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->AButtonDownAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetAButtonDown(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->BButtonDownAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetBButtonDown(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->XButtonDownAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetXButtonDown(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->YButtonDownAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetYButtonDown(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->AButtonTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetAButtonTouched(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->BButtonTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetBButtonTouched(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->XButtonTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetXButtonTouched(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->YButtonTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetYButtonTouched(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftMenuButtonDownAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetLeftMenuButtonDown(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftFrontTriggerTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetLeftFrontTriggerTouched(bValue); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftFrontTriggerAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetLeftFrontTriggerAxisValue(Value); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightFrontTriggerTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetRightFrontTriggerTouched(bValue); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightFrontTriggerAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetRightFrontTriggerAxisValue(Value); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftGripTriggerAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetLeftGripTriggerAxisValue(Value); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightGripTriggerAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetRightGripTriggerAxisValue(Value); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftThumbstickTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetLeftThumbstickTouched(bValue); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftThumbstickXAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetLeftThumbstickXAxisValue(Value); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftThumbstickYAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetLeftThumbstickYAxisValue(Value); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightThumbstickTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetRightThumbstickTouched(bValue); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightThumbstickXAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetRightThumbstickXAxisValue(Value); });

  BindAxisValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightThumbstickYAxisAction,
      [&](auto AnimInstance, float Value) { AnimInstance->SetRightThumbstickYAxisValue(Value); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->LeftPanelTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetLeftPanelTouched(bValue); });

  BindBoolValue(
      EnhancedInputComponent,
      InputActionsRigComponent->RightPanelTouchedAction,
      [&](auto AnimInstance, bool bValue) { AnimInstance->SetRightPanelTouched(bValue); });
}

USkeletalMeshComponent* UIsdkControllerVisualsRigComponent::GetAnimatedHandMeshComponent()
{
  return AnimatedHandMeshComponent;
}

UIsdkHandMeshComponent* UIsdkControllerVisualsRigComponent::GetPoseableHandMeshComponent()
{
  return PoseableHandMeshComponent;
}

UIsdkControllerMeshComponent* UIsdkControllerVisualsRigComponent::GetControllerMeshComponent()
{
  return ControllerMeshComponent;
}

void UIsdkControllerVisualsRigComponent::OnDataSourcesCreated()
{
  Super::OnDataSourcesCreated();

  ControllerMeshComponent->AttachToComponent(
      AttachedToMotionController, FAttachmentTransformRules::KeepRelativeTransform);
  ControllerMeshComponent->SetHandPointerPoseDataSource(LocalDataSources.HandPointerPose);
  ControllerMeshComponent->SetRootPoseDataSource(LocalDataSources.HandRootPose);

  AnimatedHandMeshComponent->AttachToComponent(
      AttachedToMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);

  // Create a hand data source used to drive oculus runtime derived hand posing for the
  // poseable hand mesh while in controller + hands mode
  const auto TrackingDataSubsystem = GetTrackingDataSubsystem();
  FIsdkTrackingDataSources HandDataSources;
  if (IsValid(TrackingDataSubsystem.GetObject()))
  {
    HandDataSources = IIsdkITrackingDataSubsystem::Execute_CreateHandDataSourceComponent(
        TrackingDataSubsystem.GetObject(), AttachedToMotionController);
  }

  // Transform of the hand mesh needs to be different for left vs right hands
  const auto Transform = HandType == EIsdkHandType::HandLeft
      ? IsdkXRUtils::OVR::LeftControllerHandTransform
      : IsdkXRUtils::OVR::RightControllerHandTransform;

  PoseableHandMeshComponent->AttachToComponent(
      AttachedToMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
  PoseableHandMeshComponent->SetWorldTransform(AttachedToMotionController->GetComponentTransform());
  PoseableHandMeshComponent->SetRelativeTransform(Transform);
  PoseableHandMeshComponent->SetJointsDataSource(HandDataSources.DataSourceComponent);
  PoseableHandMeshComponent->SetRootPoseDataSource(HandDataSources.DataSourceComponent);
}

void UIsdkControllerVisualsRigComponent::OnVisibilityUpdated(
    bool bTrackedVisibility,
    bool bSyntheticVisibility)
{
  Super::OnVisibilityUpdated(bTrackedVisibility, bSyntheticVisibility);

  const auto ControllerHandBehavior = UIsdkFunctionLibrary::GetControllerHandBehavior(GetWorld());

  UpdateAnimatedHandMeshVisibility(bTrackedVisibility, ControllerHandBehavior);
  UpdatePoseableHandMeshVisibility(bTrackedVisibility, ControllerHandBehavior);
  UpdateControllerMeshVisibility(bTrackedVisibility, ControllerHandBehavior);
}

void UIsdkControllerVisualsRigComponent::BindAxisValue(
    UEnhancedInputComponent* EnhancedInputComponent,
    const UInputAction* Action,
    TFunction<void(UQuestControllerAnimInstance*, float)> Lambda) const
{
  auto ControllerAnimInstance = ControllerMeshComponent
      ? Cast<UQuestControllerAnimInstance>(ControllerMeshComponent->GetAnimInstance())
      : nullptr;
  auto HandAnimInstance = AnimatedHandMeshComponent
      ? Cast<UQuestControllerAnimInstance>(AnimatedHandMeshComponent->GetAnimInstance())
      : nullptr;

  // Update on trigger event triggered
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Triggered,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, ActionInstance.GetValue().Get<float>());
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, ActionInstance.GetValue().Get<float>());
        }
      });

  // Update on trigger event completed
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Completed,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, 0.f);
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, 0.f);
        }
      });

  // Update on trigger event canceled
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Canceled,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, 0.f);
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, 0.f);
        }
      });
}

void UIsdkControllerVisualsRigComponent::BindBoolValue(
    UEnhancedInputComponent* EnhancedInputComponent,
    const UInputAction* Action,
    TFunction<void(UQuestControllerAnimInstance*, bool)> Lambda) const
{
  auto ControllerAnimInstance =
      Cast<UQuestControllerAnimInstance>(ControllerMeshComponent->GetAnimInstance());
  auto HandAnimInstance =
      Cast<UQuestControllerAnimInstance>(AnimatedHandMeshComponent->GetAnimInstance());

  // Update on trigger event started
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Started,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
      });

  // Update on trigger event completed
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Completed,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
      });

  // Update on trigger event canceled
  EnhancedInputComponent->BindActionInstanceLambda(
      Action,
      ETriggerEvent::Canceled,
      [=](const FInputActionInstance& ActionInstance)
      {
        if (ControllerAnimInstance)
        {
          Lambda(ControllerAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
        if (HandAnimInstance)
        {
          Lambda(HandAnimInstance, ActionInstance.GetValue().Get<bool>());
        }
      });
}

void UIsdkControllerVisualsRigComponent::UpdateAnimatedHandMeshVisibility(
    bool bTrackedVisibility,
    EControllerHandBehavior ControllerHandBehavior)
{
  if (!bTrackedVisibility)
  {
    AnimatedHandMeshComponent->SetVisibility(false);
    return;
  }

  bool bIsVisible;
  switch (ControllerHandBehavior)
  {
    case EControllerHandBehavior::BothAnimated:
    case EControllerHandBehavior::HandsOnlyAnimated:
      bIsVisible = true;
      break;
    default:
      bIsVisible = false;
  }

  AnimatedHandMeshComponent->SetVisibility(bIsVisible);
}

void UIsdkControllerVisualsRigComponent::UpdatePoseableHandMeshVisibility(
    bool bTrackedVisibility,
    EControllerHandBehavior ControllerHandBehavior)
{
  if (!bTrackedVisibility)
  {
    PoseableHandMeshComponent->SetVisibility(false);
    return;
  }

  bool bIsVisible;
  switch (ControllerHandBehavior)
  {
    case EControllerHandBehavior::BothProcedural:
    case EControllerHandBehavior::HandsOnlyProcedural:
      bIsVisible = true;
      break;
    default:
      bIsVisible = false;
  }

  PoseableHandMeshComponent->SetVisibility(bIsVisible);
}

void UIsdkControllerVisualsRigComponent::UpdateControllerMeshVisibility(
    bool bTrackedVisibility,
    EControllerHandBehavior ControllerHandBehavior)
{
  if (!bTrackedVisibility)
  {
    ControllerMeshComponent->SetVisibility(false);
    return;
  }

  bool bIsVisible;
  switch (ControllerHandBehavior)
  {
    case EControllerHandBehavior::HandsOnlyProcedural:
    case EControllerHandBehavior::HandsOnlyAnimated:
      bIsVisible = false;
      break;
    default:
      bIsVisible = true;
  }

  ControllerMeshComponent->SetVisibility(bIsVisible);
}
