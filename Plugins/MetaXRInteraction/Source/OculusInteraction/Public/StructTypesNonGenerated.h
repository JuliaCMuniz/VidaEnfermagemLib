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
#include "UObject/Interface.h"

// Includes needed in StructTypes.h
class IIsdkIPose;
class IIsdkIActiveState;

#include "StructTypesNonGenerated.generated.h"

UENUM(BlueprintType)
enum class EIsdkLerpState : uint8
{
  Inactive = 0,
  TransitioningTo = 1,
  TransitioningAway = 2
};

USTRUCT(BlueprintType)
struct OCULUSINTERACTION_API FIsdkPosef
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FQuat Orientation = FQuat::Identity;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FVector3f Position = FVector3f::ZeroVector;

  FTransform ToTransform() const
  {
    return FTransform(Orientation, FVector(Position));
  }

  void FromTransform(FTransform transform)
  {
    Orientation = transform.GetRotation();
    Position = (FVector3f)transform.GetLocation();
  }
};

USTRUCT(Category = InteractionSDK, BlueprintType)
struct OCULUSINTERACTION_API FIsdkInteractionRelationshipCounts
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  int NumHover{};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  int NumSelect{};
};

class UIsdkStandardEntity;
class UIsdkStandardPayload;

/**
 * Dummy payload type - we don't use the unreal type below. Payloads are looked up manually from the
 * isdk_Payload handle in the CreatePointerEventConverter.
 */
UINTERFACE(NotBlueprintable)
class OCULUSINTERACTION_API UIsdkIPayload : public UInterface
{
  GENERATED_BODY()
};
class OCULUSINTERACTION_API IIsdkIPayload
{
  GENERATED_BODY()
};

/**
 * EIsdkControllerDrivenHandPoseType is the ISDK analogue to Meta XR's
 * EOculusXRControllerDrivenHandPoseTypes, and is intended only internal use, for cross-plugin
 * communication between ISDK and Meta XR.
 */
UENUM()
enum class EIsdkXRControllerDrivenHandPoseType : uint8
{
  None = 0, // Controllers do not generate any hand poses.
  Natural, // Controller button inputs will be used to generate a normal hand pose.
  Controller, // Controller button inputs will be used to generate a hand pose holding a controller.
};

/**
 * EIsdkXRControllerType is the ISDK analogue to Meta XR's EOculusXRControllerType,
 * and is intended only internal use, for cross-plugin communication between ISDK and Meta XR.
 */
UENUM()
enum class EIsdkXRControllerType : uint8
{
  None = 0,
  MetaQuestTouch = 1,
  MetaQuestTouchPro = 2,
  MetaQuestTouchPlus = 3,
  Unknown = 0x7f,
};

/**
 * EControllerHandBehavior drives how we should present the user's hands when they are holding a
 * controller.
 */
UENUM(BlueprintType)
enum class EControllerHandBehavior : uint8
{
  BothProcedural = 0 UMETA(Hidden, DisplayName = "Controller and Hands (Procedural, Quest Only)"),
  BothAnimated UMETA(DisplayName = "Controller and Hands (Animated)"),
  ControllerOnly UMETA(DisplayName = "Controller Only"),
  HandsOnlyProcedural UMETA(DisplayName = "Hands Only (Procedural, Quest Only)"),
  HandsOnlyAnimated UMETA(DisplayName = "Hands Only (Animated)")
};
