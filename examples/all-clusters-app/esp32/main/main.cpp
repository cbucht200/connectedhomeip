/*
 *
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

#include "AppTask.h"
#include "BluetoothWidget.h"
#include "DeviceCallbacks.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "QRCodeScreen.h"
#include "ShellCommands.h"
#include "WiFiWidget.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "platform/PlatformManager.h"
#include "shell_extension/launch.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/af.h>
#include <binding-handler.h>
#include <common/CHIPDeviceManager.h>
#include <common/Esp32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <examples/platform/esp32/mode-support/static-supported-modes-manager.h>
#include <platform/ESP32/ESP32Utils.h>

#if CONFIG_HAVE_DISPLAY
#include "DeviceWithDisplay.h"
#endif

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_OPENTHREAD_ENABLED
#include <common/OpenthreadConfig.h>
#include <platform/ESP32/OpenthreadLauncher.h>
#include <platform/ThreadStackManager.h>
#endif

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::DeviceManager;
using namespace ::chip::Credentials;

// Used to indicate that an IP address has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_IP 1

const char * TAG = "all-clusters-app";

static AppDeviceCallbacks EchoCallbacks;
static AppDeviceCallbacksDelegate sAppDeviceCallbacksDelegate;
namespace {

class AppCallbacks : public AppDelegate
{
public:
    void OnCommissioningSessionStarted() override { bluetoothLED.Set(true); }
    void OnCommissioningSessionStopped() override
    {
        bluetoothLED.Set(false);
        pairingWindowLED.Set(false);
    }
    void OnCommissioningWindowOpened() override { pairingWindowLED.Set(true); }
    void OnCommissioningWindowClosed() override { pairingWindowLED.Set(false); }
};

AppCallbacks sCallbacks;

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

} // namespace

static void InitServer(intptr_t context)
{
    Esp32AppServer::Init(&sCallbacks); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config

#if !(CHIP_DEVICE_CONFIG_ENABLE_WIFI && CHIP_DEVICE_CONFIG_ENABLE_THREAD)
    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
#endif

    CHIP_ERROR err = GetAppTask().LockInit();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize app task lock, err:%" CHIP_ERROR_FORMAT, err.Format());
    }

#if CONFIG_DEVICE_TYPE_M5STACK
    SetupPretendDevices();
#endif
    err = app::Clusters::ModeSelect::StaticSupportedModesManager::getStaticSupportedModesManagerInstance().InitEndpointArray(
        FIXED_ENDPOINT_COUNT);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize endpoint array for supported-modes, err:%" CHIP_ERROR_FORMAT, err.Format());
    }
}

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default() failed: %s", esp_err_to_name(err));
        return;
    }
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "chip-esp32-all-cluster-demo starting");
    ESP_LOGI(TAG, "==================================================");

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
    OnOffCommands::GetInstance().Register();
    CASECommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize the Wi-Fi stack");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    CHIP_ERROR error              = deviceMgr.Init(&EchoCallbacks);
    DeviceCallbacksDelegate::Instance().SetAppDelegate(&sAppDeviceCallbacksDelegate);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

    ESP_LOGI(TAG, "------------------------Starting App Task---------------------------");
    error = GetAppTask().StartAppTask();
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetAppTask().StartAppTask() failed : %" CHIP_ERROR_FORMAT, error.Format());
    }
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config  = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);

    if (DeviceLayer::ThreadStackMgr().InitThreadStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize Thread stack");
        return;
    }
    if (DeviceLayer::ThreadStackMgr().StartThreadTask() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to launch Thread task");
        return;
    }
#endif

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}

bool lowPowerClusterSleep()
{
    return true;
}
