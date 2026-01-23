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

#include "Rig/IsdkPokeInteractionRigComponent.h"

#include "Core/IsdkConditionalComponentIsActive.h"
#include "Core/IsdkConditionalGroupAll.h"
#include "Interaction/IsdkPokeInteractor.h"
#include "Interaction/IsdkPokeLimiterVisual.h"

// Sets default values for this component's properties
UIsdkPokeInteractionRigComponent::UIsdkPokeInteractionRigComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
  bWantsInitializeComponent = true;
  bAutoActivate = true;

  PokeInteractor = CreateDefaultSubobject<UIsdkPokeInteractor>(TEXT("PokeInteractor"));
  PokeLimiterVisual = CreateDefaultSubobject<UIsdkPokeLimiterVisual>(TEXT("PokeLimiterVisual"));

  EnabledConditionalGroup =
      CreateDefaultSubobject<UIsdkConditionalGroupAll>(TEXT("PokeEnabledConditional"));
  IsActiveConditional =
      CreateDefaultSubobject<UIsdkConditionalComponentIsActive>(TEXT("PokeIsActiveConditional"));
}

void UIsdkPokeInteractionRigComponent::InitializeComponent()
{
  Super::InitializeComponent();

  IsActiveConditional->SetComponent(this);
  EnabledConditionalGroup->AddConditional(IsActiveConditional);
  EnabledConditionalGroup->ResolvedValueChanged.AddWeakLambda(
      this, [this](bool bIsEnabled) { HandleIsEnabledConditionalChanged(bIsEnabled); });
}

void UIsdkPokeInteractionRigComponent::BindDataSources(
    const FIsdkTrackingDataSources& DataSources,
    UIsdkSyntheticHand* SyntheticHand,
    USceneComponent* AttachToComponent,
    FName AttachToComponentSocket)
{
  PokeInteractor->SetRootPose(DataSources.HandRootPose);
  PokeInteractor->AttachToComponent(
      AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachToComponentSocket);

  // Watch for the data sources becoming enabled/disabled.
  const auto IsConnectedConditional =
      IIsdkIRootPose::Execute_GetRootPoseConnectedConditional(DataSources.HandRootPose.GetObject());
  if (ensure(IsValid(IsConnectedConditional)))
  {
    EnabledConditionalGroup->AddConditional(IsConnectedConditional);
  }

  // Wire up data sources to poke limiting. If SyntheticHand is null (i.e. if DataSources is a
  // Controller rather than a Hand), the PokeLimiterVisual will simply do nothing.
  PokeLimiterVisual->SetDataSource(DataSources.DataSourceComponent);
  PokeLimiterVisual->SetSyntheticHand(SyntheticHand);
  PokeLimiterVisual->SetPokeInteractor(PokeInteractor);
}

void UIsdkPokeInteractionRigComponent::HandleIsEnabledConditionalChanged(bool bIsEnabled)
{
  if (!bIsEnabled)
  {
    PokeInteractor->Deactivate();
    PokeLimiterVisual->Deactivate();
  }
  else
  {
    PokeInteractor->Activate();
    PokeLimiterVisual->Activate();
  }
}
