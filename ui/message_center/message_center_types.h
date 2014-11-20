// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_MESSAGE_CENTER_MESSAGE_CENTER_TYPES_H_
#define UI_MESSAGE_CENTER_MESSAGE_CENTER_TYPES_H_

namespace message_center {

enum Visibility {
  // When nothing or just toast popups are being displayed.
  VISIBILITY_TRANSIENT = 0,
  // When the message center view is being displayed.
  VISIBILITY_MESSAGE_CENTER,
  // When the settings view is being displayed.
  VISIBILITY_SETTINGS
};

}  // namespace message_center

#endif  // UI_MESSAGE_CENTER_MESSAGE_CENTER_TYPES_H_