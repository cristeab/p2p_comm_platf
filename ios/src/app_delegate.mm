#import <UIKit/UIKit.h>
#import <BackgroundTasks/BackgroundTasks.h>
#include "softphone.h"
#include <QDebug>

#define KEEP_ALIVE_INTERVAL_SEC 10

@interface QIOSApplicationDelegate : NSObject
@end

@implementation QIOSApplicationDelegate

static NSString* checkIncomingCallsTask = @"cristeab.proplife.checkincoming";

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

    //make sure the application wakes up periodically
    if (@available(iOS 13.0, *)) {
        qInfo() << "Schedule processing task";
        [self scheduleProcessingTask];
    } else {
        qWarning() << "Cannot start schedule processing task";
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
    qInfo() << "didFinishLaunchingWithOptions";

    if (@available(iOS 13.0, *)) {
        qInfo() << "configureProcessingTask";
        [self configureProcessingTask];
    }

    return YES;
}

-(void)configureProcessingTask {
    if (@available(iOS 13.0, *)) {
        [[BGTaskScheduler sharedScheduler] registerForTaskWithIdentifier:checkIncomingCallsTask
                                           usingQueue:nil
                                           launchHandler:^(BGTask *task) {
            [self handleProcessingTask:task];
        }];
        qInfo() << "Configured processing task";
    } else {
        qWarning() << "Cannot configure processing task";
    }
}

-(void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
    UIAlertView *notificationAlert = [[UIAlertView alloc] initWithTitle:@"Notification" message:notification.alertBody delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
    [notificationAlert show];
    qInfo() << "didReceiveLocalNotification";
}

-(void)scheduleProcessingTask {
    if (@available(iOS 13.0, *)) {
        NSError *error = NULL;
        // cancel existing task (if any)
        [BGTaskScheduler.sharedScheduler cancelTaskRequestWithIdentifier:checkIncomingCallsTask];
        // new task
        BGProcessingTaskRequest *request = [[BGProcessingTaskRequest alloc] initWithIdentifier:checkIncomingCallsTask];
        request.requiresNetworkConnectivity = YES;
        request.earliestBeginDate = [NSDate dateWithTimeIntervalSinceNow:5];
        BOOL success = [[BGTaskScheduler sharedScheduler] submitTaskRequest:request error:&error];
        if (!success) {
            // Errorcodes https://stackoverflow.com/a/58224050/872051
            qCritical() << "Failed to submit request" << error;
        } else {
            qInfo() << "Success submit request" << request;
        }
    } else {
        qWarning() << "Cannot schedule processing task";
    }
}

-(void)handleProcessingTask:(BGTask *)task API_AVAILABLE(ios(13.0)){
    qInfo() << "Check for incoming calls";

    auto *inst = Softphone::instance();
    if (nullptr != inst) {
        inst->zeroConf(true);
        [NSThread sleepForTimeInterval:5.0f];
        inst->zeroConf(false);
    }

    [task setTaskCompletedWithSuccess:true];
    [self scheduleProcessingTask];
}

@end

void startLocalNotification(const QString &name)
{
    qInfo() << "startLocalNotification" << name;

    UILocalNotification *notification = [[UILocalNotification alloc] init];
    notification.fireDate = [NSDate dateWithTimeIntervalSinceNow:0];
    notification.alertBody = [NSString stringWithFormat:@"Incoming call from %@", name.toNSString()];
    notification.timeZone = [NSTimeZone defaultTimeZone];
    notification.soundName = UILocalNotificationDefaultSoundName;
    notification.applicationIconBadgeNumber = 10;

    [[UIApplication sharedApplication] scheduleLocalNotification:notification];
}

QString deviceUuid()
{
    NSString *uniqueIdentifier = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
    return QString::fromNSString(uniqueIdentifier);
}
