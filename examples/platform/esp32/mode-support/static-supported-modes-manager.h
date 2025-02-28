/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <src/app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
private:
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using SemanticTag          = Structs::SemanticTagStruct::Type;
    static int mSize;

    static ModeOptionsProvider * epModeOptionsProviderList;

    void FreeSupportedModes(EndpointId endpointId) const;

    static const StaticSupportedModesManager instance;

public:
    // InitEndpointArray should be called only once in the application. Memory allocated to the
    // epModeOptionsProviderList will be needed for the lifetime of the program, so it's never deallocated.
    static CHIP_ERROR InitEndpointArray(int size);

    // DeInitEndpointArray should be called only when application need to reallocate memory of
    // epModeOptionsProviderList ( Eg. Bridges ).
    static void DeInitEndpointArray()
    {
        delete[] epModeOptionsProviderList;
        epModeOptionsProviderList = nullptr;
        mSize                     = 0;
    }

    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    void CleanUp(EndpointId endpointId) const;

    StaticSupportedModesManager() {}

    ~StaticSupportedModesManager()
    {
        for (int i = 0; i < mSize; i++)
        {
            FreeSupportedModes(i);
        }
    }

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
