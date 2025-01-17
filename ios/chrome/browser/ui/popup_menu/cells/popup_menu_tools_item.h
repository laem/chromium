// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_TOOLS_ITEM_H_
#define IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_TOOLS_ITEM_H_

#import "ios/chrome/browser/ui/popup_menu/cells/popup_menu_item.h"
#import "ios/chrome/browser/ui/table_view/cells/table_view_item.h"

// Item for a tools menu item.
@interface PopupMenuToolsItem : TableViewItem<PopupMenuItem>

// The title of the item.
@property(nonatomic, copy) NSString* title;
// Image to be displayed on the item.
@property(nonatomic, strong) UIImage* image;
// Whether the cell associated with this item should be enabled.
@property(nonatomic, assign) BOOL enabled;

@end

// Associated cell for the PopupMenuToolsItem.
@interface PopupMenuToolsCell : UITableViewCell

// Image view to display the image.
@property(nonatomic, strong, readonly) UIImageView* imageView;

// Title label for the cell.
@property(nonatomic, strong, readonly) UILabel* titleLabel;

// Returns the size this cell would use to display its content when it has a
// |title| and a maximum |width|.
+ (CGSize)sizeForWidth:(CGFloat)width title:(NSString*)title;

@end

#endif  // IOS_CHROME_BROWSER_UI_POPUP_MENU_CELLS_POPUP_MENU_TOOLS_ITEM_H_
