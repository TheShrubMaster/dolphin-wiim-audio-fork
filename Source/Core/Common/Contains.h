// Copyright 2025 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <algorithm>

namespace Common
{
  inline constexpr auto& Contains = std::ranges::contains;
  inline constexpr auto& ContainsSubrange = std::ranges::contains_subrange;
}  // namespace Common
