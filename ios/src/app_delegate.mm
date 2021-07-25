#import "UIKit/UIKit.h"
#include "softphone.h"
#include <QDebug>

@interface QIOSApplicationDelegate : NSObject
@end

@implementation QIOSApplicationDelegate

- (void)applicationWillResignActive:(UIApplication *)application
{
#pragma unused(application)

    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
    qInfo() << "applicationWillResignActive";
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
#pragma unused(application)

    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
    qInfo() << "applicationDidEnterBackground";
    auto *inst = Softphone::instance();
    if (nullptr != inst) {
        inst->zeroConf(false);
    }
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
#pragma unused(application)

    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
    qInfo() << "applicationWillEnterForeground";
    auto *inst = Softphone::instance();
    if (nullptr != inst) {
        inst->zeroConf(true);
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [launchOptions valueForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    // Override point for customization after application launch.
    return YES;
}

-(void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
    UIAlertView *notificationAlert = [[UIAlertView alloc] initWithTitle:@"Notification" message:@"This local notification" delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
    [notificationAlert show];
    qInfo() << "didReceiveLocalNotification";
}

@end

void startLocalNotification()
{
    qInfo() << "startLocalNotification";

    UILocalNotification *notification = [[UILocalNotification alloc] init];
    notification.fireDate = [NSDate dateWithTimeIntervalSinceNow:1];
    notification.alertBody = @"This is local notification!";
    notification.timeZone = [NSTimeZone defaultTimeZone];
    notification.soundName = UILocalNotificationDefaultSoundName;
    notification.applicationIconBadgeNumber = 10;

    [[UIApplication sharedApplication] scheduleLocalNotification:notification];
}
