//
//  AppDelegate.h
//  Ysflight
//
//  Created by Soji Yamakawa on 2018/06/14.
//  Copyright © 2018年 ysflight.com. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (readonly, strong) NSPersistentContainer *persistentContainer;

- (void)saveContext;


@end

