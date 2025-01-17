// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/popup_menu/cells/popup_menu_navigation_item.h"

#include "base/logging.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@implementation PopupMenuNavigationItem

@synthesize actionIdentifier = _actionIdentifier;

- (instancetype)initWithType:(NSInteger)type {
  self = [super initWithType:type];
  if (self) {
    self.cellClass = [PopupMenuNavigationCell class];
  }
  return self;
}

#pragma mark - PopupMenuItem

- (CGSize)cellSizeForWidth:(CGFloat)width {
  // TODO(crbug.com/804779): implement this.
  NOTREACHED();
  return CGSizeZero;
}

@end

@implementation PopupMenuNavigationCell
@end
