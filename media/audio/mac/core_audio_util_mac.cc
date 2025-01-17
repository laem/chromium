// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/audio/mac/core_audio_util_mac.h"

#include <utility>

#include "base/mac/mac_logging.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "media/audio/audio_manager.h"

namespace media {
namespace core_audio_mac {

namespace {

AudioObjectPropertyScope InputOutputScope(bool is_input) {
  return is_input ? kAudioObjectPropertyScopeInput
                  : kAudioObjectPropertyScopeOutput;
}

base::Optional<std::string> GetDeviceStringProperty(
    AudioObjectID device_id,
    AudioObjectPropertySelector property_selector) {
  DCHECK(AudioManager::Get()->GetTaskRunner()->BelongsToCurrentThread());
  CFStringRef property_value = nullptr;
  UInt32 size = sizeof(property_value);
  AudioObjectPropertyAddress property_address = {
      property_selector, kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster};

  OSStatus result = AudioObjectGetPropertyData(
      device_id, &property_address, 0 /* inQualifierDataSize */,
      nullptr /* inQualifierData */, &size, &property_value);
  if (result != noErr) {
    OSSTATUS_DLOG(WARNING, result)
        << "Failed to read string property " << property_selector
        << " for device " << device_id;
    return base::nullopt;
  }

  if (!property_value)
    return base::nullopt;

  std::string device_property = base::SysCFStringRefToUTF8(property_value);
  CFRelease(property_value);

  return device_property;
}

base::Optional<uint32_t> GetDeviceUint32Property(
    AudioObjectID device_id,
    AudioObjectPropertySelector property_selector,
    AudioObjectPropertyScope property_scope) {
  DCHECK(AudioManager::Get()->GetTaskRunner()->BelongsToCurrentThread());
  AudioObjectPropertyAddress property_address = {
      property_selector, property_scope, kAudioObjectPropertyElementMaster};
  UInt32 property_value;
  UInt32 size = sizeof(property_value);
  OSStatus result = AudioObjectGetPropertyData(
      device_id, &property_address, 0 /* inQualifierDataSize */,
      nullptr /* inQualifierData */, &size, &property_value);
  if (result != noErr) {
    OSSTATUS_DLOG(WARNING, result)
        << "Failed to read UInt32 property " << property_selector
        << " for device " << device_id;
    return base::nullopt;
  }
  return property_value;
}

uint32_t GetDevicePropertySize(AudioObjectID device_id,
                               AudioObjectPropertySelector property_selector,
                               AudioObjectPropertyScope property_scope) {
  DCHECK(AudioManager::Get()->GetTaskRunner()->BelongsToCurrentThread());
  AudioObjectPropertyAddress property_address = {
      property_selector, property_scope, kAudioObjectPropertyElementMaster};
  UInt32 size = 0;
  OSStatus result = AudioObjectGetPropertyDataSize(
      device_id, &property_address, 0 /* inQualifierDataSize */,
      nullptr /* inQualifierData */, &size);
  if (result != noErr) {
    OSSTATUS_DLOG(WARNING, result)
        << "Failed to read size of property " << property_selector
        << " for device " << device_id;
    return 0;
  }
  return size;
}

std::vector<AudioObjectID> GetAudioDeviceIDs(
    AudioObjectID audio_object_id,
    AudioObjectPropertySelector property_selector) {
  DCHECK(AudioManager::Get()->GetTaskRunner()->BelongsToCurrentThread());
  AudioObjectPropertyAddress property_address = {
      property_selector, kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster};
  UInt32 size = 0;
  OSStatus result = AudioObjectGetPropertyDataSize(
      audio_object_id, &property_address, 0 /* inQualifierDataSize */,
      nullptr /* inQualifierData */, &size);
  if (result != noErr) {
    OSSTATUS_DLOG(WARNING, result)
        << "Failed to read size of property " << property_selector
        << " for device/object " << audio_object_id;
    return {};
  }

  if (size == 0)
    return {};

  size_t device_count = size / sizeof(AudioObjectID);
  // Get the array of device ids for all the devices, which includes both
  // input devices and output devices.
  std::vector<AudioObjectID> device_ids(device_count);
  result = AudioObjectGetPropertyData(
      audio_object_id, &property_address, 0 /* inQualifierDataSize */,
      nullptr /* inQualifierData */, &size, device_ids.data());
  if (result != noErr) {
    OSSTATUS_DLOG(WARNING, result)
        << "Failed to read devuce IDs for property " << property_selector
        << " for device/object " << audio_object_id;
    return {};
  }

  return device_ids;
}

base::Optional<std::string> GetDeviceName(AudioObjectID device_id) {
  return GetDeviceStringProperty(device_id, kAudioObjectPropertyName);
}

base::Optional<std::string> GetDeviceModel(AudioObjectID device_id) {
  return GetDeviceStringProperty(device_id, kAudioDevicePropertyModelUID);
}

base::Optional<uint32_t> GetDeviceTransportType(AudioObjectID device_id) {
  return GetDeviceUint32Property(device_id, kAudioDevicePropertyTransportType,
                                 kAudioObjectPropertyScopeGlobal);
}

bool ModelContainsVidPid(const std::string& model) {
  return model.size() > 10 && model[model.size() - 5] == ':' &&
         model[model.size() - 10] == ':';
}

std::string UsbVidPidFromModel(const std::string& model) {
  return ModelContainsVidPid(model)
             ? base::ToLowerASCII(model.substr(model.size() - 9))
             : std::string();
}

std::string TransportTypeToString(uint32_t transport_type) {
  switch (transport_type) {
    case kAudioDeviceTransportTypeBuiltIn:
      return "Built-in";
    case kAudioDeviceTransportTypeAggregate:
      return "Aggregate";
    case kAudioDeviceTransportTypeAutoAggregate:
      return "AutoAggregate";
    case kAudioDeviceTransportTypeVirtual:
      return "Virtual";
    case kAudioDeviceTransportTypePCI:
      return "PCI";
    case kAudioDeviceTransportTypeUSB:
      return "USB";
    case kAudioDeviceTransportTypeFireWire:
      return "FireWire";
    case kAudioDeviceTransportTypeBluetooth:
      return "Bluetooth";
    case kAudioDeviceTransportTypeBluetoothLE:
      return "Bluetooth LE";
    case kAudioDeviceTransportTypeHDMI:
      return "HDMI";
    case kAudioDeviceTransportTypeDisplayPort:
      return "DisplayPort";
    case kAudioDeviceTransportTypeAirPlay:
      return "AirPlay";
    case kAudioDeviceTransportTypeAVB:
      return "AVB";
    case kAudioDeviceTransportTypeThunderbolt:
      return "Thunderbolt";
    case kAudioDeviceTransportTypeUnknown:
    default:
      return std::string();
  }
}

}  // namespace

std::vector<AudioObjectID> GetAllAudioDeviceIDs() {
  return GetAudioDeviceIDs(kAudioObjectSystemObject,
                           kAudioHardwarePropertyDevices);
}

std::vector<AudioObjectID> GetRelatedDeviceIDs(AudioObjectID device_id) {
  return GetAudioDeviceIDs(device_id, kAudioDevicePropertyRelatedDevices);
}

base::Optional<std::string> GetDeviceUniqueID(AudioObjectID device_id) {
  return GetDeviceStringProperty(device_id, kAudioDevicePropertyDeviceUID);
}

base::Optional<std::string> GetDeviceLabel(AudioObjectID device_id,
                                           bool is_input) {
  base::Optional<std::string> device_label = GetDeviceName(device_id);
  if (!device_label)
    return base::nullopt;

  std::string suffix;
  base::Optional<uint32_t> transport_type = GetDeviceTransportType(device_id);
  if (transport_type) {
    if (*transport_type == kAudioDeviceTransportTypeUSB) {
      base::Optional<std::string> model = GetDeviceModel(device_id);
      if (model) {
        suffix = UsbVidPidFromModel(*model);
      }
    } else {
      suffix = TransportTypeToString(*transport_type);
    }
  }

  if (!suffix.empty())
    *device_label += " (" + suffix + ")";

  return device_label;
}

uint32_t GetNumStreams(AudioObjectID device_id, bool is_input) {
  return GetDevicePropertySize(device_id, kAudioDevicePropertyStreams,
                               InputOutputScope(is_input));
}

}  // namespace core_audio_mac
}  // namespace media
