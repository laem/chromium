// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_COMMANDS_POPUP_MENU_COMMANDS_H_
#define IOS_CHROME_BROWSER_UI_COMMANDS_POPUP_MENU_COMMANDS_H_

#import <Foundation/Foundation.h>

// Commands for the popup menu.
@protocol PopupMenuCommands

// Shows the navigation history popup containing the back history.
- (void)showNavigationHistoryBackPopupMenu;
// Shows the navigation history popup containing the forward history.
- (void)showNavigationHistoryForwardPopupMenu;
// Shows the tools menu.
- (void)showToolsMenuPopup;
// Shows the popup for the tab grid button.
- (void)showTabGridButtonPopup;
// Shows the popup for the search button.
- (void)searchButtonPopup;
// Dismisses the currently presented popup.
- (void)dismissPopupMenu;

@end

#endif  // IOS_CHROME_BROWSER_UI_COMMANDS_POPUP_MENU_COMMANDS_H_
