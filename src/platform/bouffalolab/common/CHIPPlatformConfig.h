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

#pragma once

#include <stdint.h>

// ==================== General Platform Adaptations ====================
#define ChipDie() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY 0x01

// ==================== Security Adaptations ====================
//#define CHIP_CONFIG_SHA256_CONTEXT_SIZE sizeof(bl_sha_ctx_t)
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE ((1 + 5 + 18 + 16 + 16) * sizeof(unsigned int))

#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS 1
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 0

#define CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG 1
#define CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM 0

#define CHIP_CONFIG_ENABLE_PASE_INITIATOR 0
#define CHIP_CONFIG_ENABLE_PASE_RESPONDER 1
#define CHIP_CONFIG_ENABLE_CASE_INITIATOR 1

#define CHIP_CONFIG_SUPPORT_PASE_CONFIG0 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG1 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG2 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG3 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG4 1

#define CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR 0

#define CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 0

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT 6
#endif // CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_MAX_INTERFACES
#define CHIP_CONFIG_MAX_INTERFACES 4
#endif // CHIP_CONFIG_MAX_INTERFACES

#ifndef CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS
#define CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS 4
#endif // CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 6 // 5 fabrics + 1 for rotation slack
#endif

// ==================== Security Configuration Overrides ====================

#ifndef CHIP_CONFIG_DEBUG_CERT_VALIDATION
#define CHIP_CONFIG_DEBUG_CERT_VALIDATION 0
#endif // CHIP_CONFIG_DEBUG_CERT_VALIDATION

// ==================== FreeRTOS Configuration Overrides ====================
#ifndef CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
#define CHIP_CONFIG_FREERTOS_USE_STATIC_TASK 1
#endif

#ifndef CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
#define CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE 1
#endif
