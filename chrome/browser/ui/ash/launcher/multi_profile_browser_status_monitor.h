// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_LAUNCHER_MULTI_PROFILE_BROWSER_STATUS_MONITOR_H_
#define CHROME_BROWSER_UI_ASH_LAUNCHER_MULTI_PROFILE_BROWSER_STATUS_MONITOR_H_

#include "chrome/browser/ui/ash/launcher/browser_status_monitor.h"

// MultiProfileBrowserStatusMonitor uses mainly the BrowserStatusMonitormonitor
// with the addition that it creates and destroys launcher items for windowed
// V1 apps - upon creation as well as upon user switch.
class MultiProfileBrowserStatusMonitor : public BrowserStatusMonitor {
 public:
  explicit MultiProfileBrowserStatusMonitor(
      ChromeLauncherController* launcher_controller);
  virtual ~MultiProfileBrowserStatusMonitor();

  // BrowserStatusMonitor overrides.
  virtual void ActiveUserChanged(const std::string& user_email) OVERRIDE;
  virtual void AddV1AppToShelf(Browser* browser) OVERRIDE;
  virtual void RemoveV1AppFromShelf(Browser* browser) OVERRIDE;

 private:
  typedef std::vector<Browser*> AppList;
  AppList app_list_;

  // The launcher controller which is associated with this object.
  ChromeLauncherController* launcher_controller_;

  DISALLOW_COPY_AND_ASSIGN(MultiProfileBrowserStatusMonitor);
};

#endif  // CHROME_BROWSER_UI_ASH_LAUNCHER_MULTI_PROFILE_BROWSER_STATUS_MONITOR_H_
