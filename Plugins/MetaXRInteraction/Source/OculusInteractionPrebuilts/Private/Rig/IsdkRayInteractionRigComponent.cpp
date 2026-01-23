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

#include "Rig/IsdkRayInteractionRigComponent.h"

#include "IXRTrackingSystem.h"
#include "Interaction/IsdkRayInteractor.h"
#include "Core/IsdkConditionalGroupAll.h"
#include "InteractorVisuals/IsdkRayVisualFunctionLibrary.h"
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "Utilities/IsdkXRUtils.h"

UIsdkRayInteractionRigComponent::UIsdkRayInteractionRigComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
  bWantsInitializeComponent = true;
  bAutoActivate = true;

  RayInteractor = CreateDefaultSubobject<UIsdkRayInteractor>(TEXT("RayInteractor"));
  EnabledConditionalGroup =
      CreateDefaultSubobject<UIsdkConditionalGroupAll>(TEXT("RayEnabledConditionalAll"));
  IsActiveConditional =
      CreateDefaultSubobject<UIsdkConditionalComponentIsActive>(TEXT("RayIsActiveConditional"));

  // Update default ray length to "infinity"
  FIsdkRayInteractor_Config RayConfig = RayInteractor->GetConfig();
  RayConfig.MaxRayLength = 0;
  RayInteractor->SetConfig(RayConfig);

  RayVisualClass = UIsdkRayVisualComponent::StaticClass();
}

void UIsdkRayInteractionRigComponent::InitializeComponent()
{
  Super::InitializeComponent();

  IsActiveConditional->SetComponent(this);
  EnabledConditionalGroup->AddConditional(IsActiveConditional);
  EnabledConditionalGroup->ResolvedValueChanged.AddWeakLambda(
      this, [this](bool bIsEnabled) { HandleIsEnabledConditionalChanged(bIsEnabled); });
}

void UIsdkRayInteractionRigComponent::BindDataSources(
    const FIsdkTrackingDataSources& DataSources,
    TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceIn,
    USceneComponent* AttachToComponent,
    FName AttachToComponentSocket)
{
  RayInteractor->AttachToComponent(
      AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachToComponentSocket);
  RayInteractor->SetHandPointerPose(DataSources.HandPointerPose);

  if (IsValid(HmdDataSourceIn.GetObject()))
  {
    HmdDataSource = HmdDataSourceIn;
  }

  CreateRayVisual(AttachToComponent->GetAttachParentActor());
  if (IsValid(HmdDataSource.GetObject()) && IsValid(RayVisual))
  {
    RayVisual->HmdDataSource = HmdDataSource;
  }

  // Watch for the data sources becoming enabled/disabled.
  const auto IsConnectedConditional =
      IIsdkIRootPose::Execute_GetRootPoseConnectedConditional(DataSources.HandRootPose.GetObject());
  if (ensure(IsValid(IsConnectedConditional)))
  {
    EnabledConditionalGroup->AddConditional(IsConnectedConditional);
  }
}

void UIsdkRayInteractionRigComponent::UpdateRayVisual()
{
  UIsdkRayVisualFunctionLibrary::UpdateRayVisualWithRayInteractor(
      RayVisual, RayInteractor, GetRaySelectStrength());
}

void UIsdkRayInteractionRigComponent::BindInputActions(
    UEnhancedInputComponent* EnhancedInputComponent,
    UInputAction* InRaySelectAction,
    UInputAction* InRaySelectStrengthAction)
{
  if (ensureMsgf(
          IsValid(InRaySelectStrengthAction),
          TEXT(
              "FIsdkRigComponentRayInteraction::BindInputActions RaySelectStrengthAction argument was invalid: the RayInteractor visuals will not respond properly.")))
  {
    RaySelectStrengthIAValue = &EnhancedInputComponent->BindActionValue(InRaySelectStrengthAction);
  }

  if (ensureMsgf(
          IsValid(InRaySelectAction),
          TEXT(
              "FIsdkRigComponentRayInteraction::BindInputActions InRaySelectAction argument was invalid: the RayInteractor will not have a select action.")))
  {
    EnhancedInputComponent->BindAction(
        InRaySelectAction,
        ETriggerEvent::Started,
        RayInteractor.Get(),
        &UIsdkRayInteractor::Select);
    EnhancedInputComponent->BindAction(
        InRaySelectAction,
        ETriggerEvent::Completed,
        RayInteractor.Get(),
        &UIsdkRayInteractor::Unselect);
    EnhancedInputComponent->BindAction(
        InRaySelectAction,
        ETriggerEvent::Canceled,
        RayInteractor.Get(),
        &UIsdkRayInteractor::Unselect);
  }
}

float UIsdkRayInteractionRigComponent::GetRaySelectStrength()
{
  if (IsdkXRUtils::IsUsingOpenXR())
  {
    return PinchRecognizer->GetPinchStrength();
  }

  if (RaySelectStrengthIAValue)
  {
    return RaySelectStrengthIAValue->GetValue().Get<float>();
  }

  return 0.0f;
}

void UIsdkRayInteractionRigComponent::CreateRayVisual(AActor* AttachParent)
{
  if (!IsValid(RayVisualClass) || !IsValid(AttachParent))
  {
    return;
  }

  if (IsValid(RayVisual))
  {
    RayVisual->DestroyComponent();
  }

  RayVisual = Cast<UIsdkRayVisualComponent>(
      GetOwner()->AddComponentByClass(RayVisualClass.Get(), true, FTransform::Identity, false));

  RayVisual->AttachToComponent(
      AttachParent->GetDefaultAttachComponent(),
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));

  RayInteractor->GetUpdatedEventDelegate().AddUniqueDynamic(
      this, &UIsdkRayInteractionRigComponent::UpdateRayVisual);
}

void UIsdkRayInteractionRigComponent::HandleIsEnabledConditionalChanged(bool bIsEnabled)
{
  if (!bIsEnabled)
  {
    RayInteractor->Deactivate();
    if (IsValid(RayVisual))
    {
      RayVisual->SetVisibility(false);
    }
  }
  else
  {
    RayInteractor->Activate();
  }
}
