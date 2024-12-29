// Copyright 2010 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Core/HW/WiimoteEmu/Extension/Extension.h"

#include <array>
#include <string>

#include "Common/ChunkFile.h"
#include "Common/CommonTypes.h"
#include "Common/I2C.h"
#include "Core/HW/WiimoteEmu/Encryption.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"

#ifdef _MSC_VER
#pragma warning(disable : 4250)  // C4250 inherits via dominance - intended behavior here
#endif

namespace WiimoteEmu
{
struct DesiredExtensionState;

class Extension : public ControllerEmu::EmulatedController, public virtual Common::I2CSlave
{
public:
  explicit Extension(const char* name);
  Extension(const char* config_name, const char* display_name);

  std::string GetName() const override;
  std::string GetDisplayName() const override;

  InputConfig* GetConfig() const override;

  // Used by the wiimote to detect extension changes.
  // The normal extensions short this pin so it's always connected,
  // but M+ does some tricks with it during activation.
  virtual bool ReadDeviceDetectPin() const = 0;

  virtual void Reset() = 0;
  virtual void DoState(PointerWrap& p) = 0;
  virtual void BuildDesiredExtensionState(DesiredExtensionState* target_state) = 0;
  virtual void Update(const DesiredExtensionState& target_state) = 0;

private:
  const char* const m_config_name;
  const char* const m_display_name;
};

class None : public Extension
{
public:
  explicit None();

private:
  bool ReadDeviceDetectPin() const override;
  void BuildDesiredExtensionState(DesiredExtensionState* target_state) override;
  void Update(const DesiredExtensionState& target_state) override;
  void Reset() override;
  void DoState(PointerWrap& p) override;

  bool StartWrite(u8 slave_addr) override;
  bool StartRead(u8 slave_addr) override;
  void Stop() override;
  std::optional<u8> ReadByte() override;
  bool WriteByte(u8 value) override;
};

// This class provides the encryption and initialization behavior of most extensions.
class EncryptedExtension : public Extension, public Common::I2CSlaveAutoIncrementing
{
public:
  static constexpr u8 I2C_ADDR = 0x52;
  static constexpr int CONTROLLER_DATA_BYTES = 21;

  explicit EncryptedExtension(const char* name)
      : Extension(name), I2CSlaveAutoIncrementing(I2C_ADDR)
  {
  }
  EncryptedExtension(const char* config_name, const char* display_name)
      : Extension(config_name, display_name), I2CSlaveAutoIncrementing(I2C_ADDR)
  {
  }

  // TODO: This is public for TAS reasons.
  // TODO: TAS handles encryption poorly.
  EncryptionKey ext_key;

  using I2CSlaveAutoIncrementing::ReadByte;
  using I2CSlaveAutoIncrementing::StartRead;
  using I2CSlaveAutoIncrementing::StartWrite;
  using I2CSlaveAutoIncrementing::Stop;
  using I2CSlaveAutoIncrementing::WriteByte;

  static constexpr int CALIBRATION_CHECKSUM_BYTES = 2;

#pragma pack(push, 1)
  struct Register
  {
    // 21 bytes of possible extension data
    u8 controller_data[CONTROLLER_DATA_BYTES];

    u8 unknown2[11];

    // address 0x20
    std::array<u8, 0x10> calibration;
    u8 unknown3[0x10];

    // address 0x40
    std::array<u8, 0x10> encryption_key_data;
    u8 unknown4[0xA0];

    // address 0xF0
    u8 encryption;
    u8 unknown5[0x9];

    // address 0xFA
    std::array<u8, 6> identifier;
  };
#pragma pack(pop)

  static_assert(0x100 == sizeof(Register));

protected:
  Register m_reg = {};

  void Reset() override;
  void DoState(PointerWrap& p) override;

  virtual void UpdateEncryptionKey() = 0;

private:
  bool m_is_key_dirty = true;

  static constexpr u8 ENCRYPTION_ENABLED = 0xaa;

  bool ReadDeviceDetectPin() const override;

  u8 ReadByte(u8 addr) override;
  void WriteByte(u8 addr, u8 value) override;
};

class Extension1stParty : public EncryptedExtension
{
protected:
  using EncryptedExtension::EncryptedExtension;

private:
  void UpdateEncryptionKey() final override;
};

class Extension3rdParty : public EncryptedExtension
{
protected:
  using EncryptedExtension::EncryptedExtension;

private:
  void UpdateEncryptionKey() final override;
};

}  // namespace WiimoteEmu
