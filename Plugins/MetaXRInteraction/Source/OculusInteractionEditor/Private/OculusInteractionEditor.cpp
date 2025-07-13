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

#include "OculusInteractionEditor.h"

#include "IsdkRuntimeSettings.h"
#include "ISettingsModule.h"

class FOculusInteractionEditor : public IOculusInteractionEditor
{
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};

#define LOCTEXT_NAMESPACE "FOculusInteractionEditorModule"

IMPLEMENT_MODULE(FOculusInteractionEditor, OculusInteractionEditor)

void FOculusInteractionEditor::StartupModule()
{
  // This code will execute after your module is loaded into memory (but after global variables are
  // initialized, of course.)

  // register settings
  ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

  if (SettingsModule != nullptr)
  {
    SettingsModule->RegisterSettings(
        "Project",
        "Plugins",
        "Isdk",
        LOCTEXT("IsdkSettingsName", "Meta XR - InteractionSDK"),
        LOCTEXT("IsdkSettingsDescription", "Project settings for Meta InteractionSdk plugin"),
        GetMutableDefault<UIsdkRuntimeSettings>());
  }
  // Register our Custom Asset Actions
  IsdkHandPoseDataAssetTypeActions = MakeShared<FIsdkHandPoseDataAssetTypeActions>();
  FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(
      IsdkHandPoseDataAssetTypeActions.ToSharedRef());
}

void FOculusInteractionEditor::ShutdownModule()
{
  // unregister settings
  ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

  if (SettingsModule != nullptr)
  {
    SettingsModule->UnregisterSettings("Project", "Plugins", "Isdk");
  }

  if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
    return;
  FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(
      IsdkHandPoseDataAssetTypeActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
