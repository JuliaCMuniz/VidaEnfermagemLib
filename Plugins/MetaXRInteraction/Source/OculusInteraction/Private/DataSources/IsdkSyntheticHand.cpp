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

#include "DataSources/IsdkSyntheticHand.h"

#include "StructTypesPrivate.h"
#include "ApiImpl.h"
#include "IsdkChecks.h"

using isdk::api::IHandDataModifier;
using isdk::api::IHandDataSource;
using isdk::api::SyntheticHand;
using isdk::api::SyntheticHandPtr;

namespace isdk::api::helper
{
class FSyntheticHandImpl : public FApiImpl<SyntheticHand, SyntheticHandPtr>
{
 public:
  explicit FSyntheticHandImpl(std::function<SyntheticHandPtr()> CreateFn)
      : FApiImpl(std::move(CreateFn))
  {
  }
};
} // namespace isdk::api::helper

UIsdkSyntheticHand::UIsdkSyntheticHand()
{
  SyntheticHandImpl = MakePimpl<isdk::api::helper::FSyntheticHandImpl, EPimplPtrMode::NoCopy>(
      [this]() -> SyntheticHandPtr
      {
        FIsdkSyntheticHand_Config Config{};
        isdk_SyntheticHand_Config IsdkConfig;
        StructTypesUtils::Copy(Config, IsdkConfig);

        // If FromDataSource is set, validate its handle. Otherwise, create our instance with no
        // 'From', with the expectation that it will be set in the future.
        isdk_IHandDataSource* FromDataSourceHandle{};
        TryGetApiFromDataSource(FromDataSourceHandle);

        auto Instance =
            SyntheticHand::create(FromDataSourceHandle, bRecursiveUpdate ? 1 : 0, IsdkConfig);
        if (!UIsdkChecks::EnsureMsgfApiInstanceIsValid(Instance.IsValid(), this))
        {
          return nullptr;
        }

        return Instance;
      });
}

void UIsdkSyntheticHand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  SyntheticHandImpl->DestroyInstance();
  Super::EndPlay(EndPlayReason);
}

void UIsdkSyntheticHand::BeginDestroy()
{
  Super::BeginDestroy();
  SyntheticHandImpl.Reset();
}

bool UIsdkSyntheticHand::IsApiInstanceValid() const
{
  return SyntheticHandImpl->IsInstanceValid();
}

IHandDataSource* UIsdkSyntheticHand::GetApiIHandDataSource()
{
  return GetApiSyntheticHand();
}

IHandDataModifier* UIsdkSyntheticHand::GetApiIHandDataModifier()
{
  return GetApiSyntheticHand();
}

SyntheticHand* UIsdkSyntheticHand::GetApiSyntheticHand()
{
  return SyntheticHandImpl->GetOrCreateInstance();
}
