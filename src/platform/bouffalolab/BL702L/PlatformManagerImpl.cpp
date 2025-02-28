/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#include <lwip/tcpip.h>

extern "C" {
#include <bl_sec.h>
#include <openthread_port.h>
#include <ot_utils_ext.h>
}

namespace chip {
namespace DeviceLayer {

extern "C" void (*ot_otrNotifyEvent_ptr)(ot_system_event_t sevent);
extern "C" void bl_rand_stream(unsigned char *, int);
extern "C" void otrNotifyEvent(ot_system_event_t sevent);

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    bl_rand_stream(output, len);
    if (olen)
    {
        *olen = len;
    }

    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;
    TaskHandle_t backup_eventLoopTask;

    // Initialize the configuration system.
    err = Internal::BLConfig::Init();
    SuccessOrExit(err);

    otRadio_opt_t opt;

    opt.byte            = 0;
    opt.bf.isCoexEnable = true;

    ot_utils_init();
    ot_otrNotifyEvent_ptr = otrNotifyEvent;

    ot_alarmInit();
    ot_radioInit(opt);

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    // Initialize LwIP.
    tcpip_init(NULL, NULL);

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    /** weiyin, backup mEventLoopTask which is reset in _InitChipStack */
    backup_eventLoopTask = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask;
    err                  = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask = backup_eventLoopTask;

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
