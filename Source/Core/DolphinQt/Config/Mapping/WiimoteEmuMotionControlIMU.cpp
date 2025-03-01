// Copyright 2019 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DolphinQt/Config/Mapping/WiimoteEmuMotionControlIMU.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/Extension/Nunchuk.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"

#include "DolphinQt/Config/ControllerInterface/ControllerInterfaceWindow.h"
#include "DolphinQt/Config/Mapping/MappingWindow.h"
#include "DolphinQt/Config/Mapping/WiimoteEmuGeneral.h"
#include "DolphinQt/QtUtils/SetWindowDecorations.h"

#include "InputCommon/InputConfig.h"

WiimoteEmuMotionControlIMU::WiimoteEmuMotionControlIMU(MappingWindow* window,
                                                       WiimoteEmuGeneral* wm_emu_general)
    : MappingWidget(window)
{
  auto* const warning_layout = new QHBoxLayout;
  auto* const warning_label =
      new QLabel(tr("WARNING: The controls under Accelerometer and Gyroscope are designed to "
                    "interface directly with motion sensor hardware. They are not intended for "
                    "mapping traditional buttons, triggers or axes. You might need to configure "
                    "alternate input sources before using these controls."));
  warning_label->setWordWrap(true);
  auto* const warning_input_sources_button = new QPushButton(tr("Alternate Input Sources"));
  warning_layout->addWidget(warning_label, 1);
  warning_layout->addWidget(warning_input_sources_button, 0, Qt::AlignRight);
  connect(warning_input_sources_button, &QPushButton::clicked, this, [this] {
    auto* const w = new ControllerInterfaceWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->setWindowModality(Qt::WindowModality::WindowModal);
    SetQWidgetWindowDecorations(w);
    w->show();
  });

  auto* const groups_layout = new QHBoxLayout;

  auto* const col0 = new QVBoxLayout;
  groups_layout->addLayout(col0);
  col0->addWidget(
      CreateGroupBox(Wiimote::GetWiimoteGroup(GetPort(), WiimoteEmu::WiimoteGroup::IRPassthrough)));
  col0->addWidget(
      CreateGroupBox(Wiimote::GetWiimoteGroup(GetPort(), WiimoteEmu::WiimoteGroup::IMUPoint)));

  groups_layout->addWidget(
      CreateGroupBox(Wiimote::GetWiimoteGroup(GetPort(), WiimoteEmu::WiimoteGroup::IMUGyroscope)));

  auto* const col2 = new QVBoxLayout;
  groups_layout->addLayout(col2);
  col2->addWidget(CreateGroupBox(
      Wiimote::GetWiimoteGroup(GetPort(), WiimoteEmu::WiimoteGroup::IMUAccelerometer)));
  auto* const nunchuk_group = CreateGroupBox(
      tr("Nunchuk Accelerometer"),
      Wiimote::GetNunchukGroup(GetPort(), WiimoteEmu::NunchukGroup::IMUAccelerometer));
  col2->addWidget(nunchuk_group);
  connect(
      wm_emu_general, &WiimoteEmuGeneral::AttachmentChanged, this, [nunchuk_group](int attachment) {
        nunchuk_group->setVisible(WiimoteEmu::ExtensionNumber(attachment) == WiimoteEmu::NUNCHUK);
      });

  auto* const main_layout = new QVBoxLayout{this};
  main_layout->addLayout(warning_layout);
  main_layout->addLayout(groups_layout);
}

void WiimoteEmuMotionControlIMU::LoadSettings()
{
  Wiimote::LoadConfig();
}

void WiimoteEmuMotionControlIMU::SaveSettings()
{
  Wiimote::GetConfig()->SaveConfig();
}

InputConfig* WiimoteEmuMotionControlIMU::GetConfig()
{
  return Wiimote::GetConfig();
}
