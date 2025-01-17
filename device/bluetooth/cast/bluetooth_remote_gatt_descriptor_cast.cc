// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/cast/bluetooth_remote_gatt_descriptor_cast.h"

#include <stdint.h>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "chromecast/device/bluetooth/bluetooth_util.h"
#include "chromecast/device/bluetooth/le/remote_descriptor.h"
#include "device/bluetooth/bluetooth_gatt_service.h"
#include "device/bluetooth/cast/bluetooth_remote_gatt_characteristic_cast.h"

namespace device {

BluetoothRemoteGattDescriptorCast::BluetoothRemoteGattDescriptorCast(
    BluetoothRemoteGattCharacteristicCast* characteristic,
    scoped_refptr<chromecast::bluetooth::RemoteDescriptor> remote_descriptor)
    : characteristic_(characteristic),
      remote_descriptor_(remote_descriptor),
      weak_factory_(this) {}

BluetoothRemoteGattDescriptorCast::~BluetoothRemoteGattDescriptorCast() {}

std::string BluetoothRemoteGattDescriptorCast::GetIdentifier() const {
  return chromecast::bluetooth::util::UuidToString(remote_descriptor_->uuid());
}

BluetoothUUID BluetoothRemoteGattDescriptorCast::GetUUID() const {
  return BluetoothUUID(
      chromecast::bluetooth::util::UuidToString(remote_descriptor_->uuid()));
}

BluetoothGattCharacteristic::Permissions
BluetoothRemoteGattDescriptorCast::GetPermissions() const {
  NOTIMPLEMENTED();
  return BluetoothGattCharacteristic::Permissions();
}

const std::vector<uint8_t>& BluetoothRemoteGattDescriptorCast::GetValue()
    const {
  return value_;
}

BluetoothRemoteGattCharacteristic*
BluetoothRemoteGattDescriptorCast::GetCharacteristic() const {
  return characteristic_;
}

void BluetoothRemoteGattDescriptorCast::ReadRemoteDescriptor(
    const ValueCallback& callback,
    const ErrorCallback& error_callback) {
  remote_descriptor_->Read(
      base::BindOnce(&BluetoothRemoteGattDescriptorCast::OnReadRemoteDescriptor,
                     weak_factory_.GetWeakPtr(), callback, error_callback));
}

void BluetoothRemoteGattDescriptorCast::WriteRemoteDescriptor(
    const std::vector<uint8_t>& new_value,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  remote_descriptor_->Write(
      new_value,
      base::BindOnce(
          &BluetoothRemoteGattDescriptorCast::OnWriteRemoteDescriptor,
          weak_factory_.GetWeakPtr(), new_value, callback, error_callback));
}

void BluetoothRemoteGattDescriptorCast::OnReadRemoteDescriptor(
    const ValueCallback& callback,
    const ErrorCallback& error_callback,
    bool success,
    const std::vector<uint8_t>& result) {
  if (success) {
    value_ = result;
    callback.Run(result);
    return;
  }
  error_callback.Run(BluetoothGattService::GATT_ERROR_FAILED);
}

void BluetoothRemoteGattDescriptorCast::OnWriteRemoteDescriptor(
    const std::vector<uint8_t>& written_value,
    const base::Closure& callback,
    const ErrorCallback& error_callback,
    bool success) {
  if (success) {
    value_ = written_value;
    callback.Run();
    return;
  }
  error_callback.Run(BluetoothGattService::GATT_ERROR_FAILED);
}

}  // namespace device
