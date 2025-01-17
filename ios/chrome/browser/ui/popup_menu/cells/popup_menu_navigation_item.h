// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_NAVIGATION_ITEM_H_
#define IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_NAVIGATION_ITEM_H_

#import "ios/chrome/browser/ui/popup_menu/cells/popup_menu_item.h"
#import "ios/chrome/browser/ui/table_view/cells/table_view_item.h"

// Item used to display an item for a navigation menu.
@interface PopupMenuNavigationItem : TableViewItem<PopupMenuItem>
@end

// Associated cell for a PopupMenuNavigationItem.
@interface PopupMenuNavigationCell : UITableViewCell
@end

#endif  // IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_NAVIGATION_ITEM_H_
