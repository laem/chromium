// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_TABLE_VIEW_CELLS_TABLE_VIEW_SIGNIN_PROMO_ITEM_H_
#define IOS_CHROME_BROWSER_UI_TABLE_VIEW_CELLS_TABLE_VIEW_SIGNIN_PROMO_ITEM_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/browser/ui/table_view/cells/table_view_item.h"

@class SigninPromoView;
@class SigninPromoViewConfigurator;
@protocol SigninPromoViewDelegate;

// TableViewItem for SignIn Promo Cell.
@interface TableViewSigninPromoItem : TableViewItem
// SigninPromoViewConfigurator used to configure the SigninPromoView.
@property(nonatomic, weak) SigninPromoViewConfigurator* configurator;
// SigninPromoViewDelegate to perform SigninPromoView actions.
@property(nonatomic, weak) id<SigninPromoViewDelegate> delegate;
// Text being displayed.
@property(nonatomic, readwrite, strong) NSString* text;
@end

// UITableViewCell that contains a SignInPromoView.
@interface TableViewSigninPromoCell : UITableViewCell
// The SigninPromoView contained by this Cell.
@property(nonatomic, strong) SigninPromoView* signinPromoView;
@end

#endif  // IOS_CHROME_BROWSER_UI_TABLE_VIEW_CELLS_TABLE_VIEW_SIGNIN_PROMO_ITEM_H_
