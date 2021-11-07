//
//  main.c
//  Sophond
//
//  Created by jiaxw on 2021/11/7.
//  Copyright (c) 2021 ___ORGANIZATIONNAME___. All rights reserved.
//

#include <UIKit/UIKit.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <notify.h>
#include <stdlib.h>
#include <objc/runtime.h>
#include "SBApplicationController.h"
#include "SBUIController.h"
#include "GSEvent.h" // for: GSEventRecord, GSCurrentEventTimestamp(), GSSendSystemEvent()
#import "RuntimeInvoker.h"

const char *kSpringBoardServiceLib = "/System/Library/PrivateFrameworks/SpringBoardServices.framework/SpringBoardServices";
const char *kBiometricKitLib = "./System/Library/PrivateFrameworks/BiometricKit.framework/BiometricKit";
const char *kAppleAccountLib = "/System/Library/PrivateFrameworks/AppleAccount.framework/AppleAccount";

//void SBSSuspendFrontmostApplication();
void (*g_ptrSBSSuspendFrontmostApplication)();

CFStringRef (*g_ptrSBSCopyFrontmostApplicationDisplayIdentifier)();

int (*g_ptrSBSOpenSensitiveURLAndUnlock)(CFURLRef url, char flags);

int (*g_SBSLaunchApplicationWithIdentifier)(CFStringRef identifier, Boolean suspended);

//BOOL SBSProcessIDForDisplayIdentifier(CFStringRef identifier, pid_t *pid);
BOOL (*g_ptrSBSProcessIDForDisplayIdentifier)(CFStringRef identifier, pid_t *pid);

void initSpringService(void);
void initSpringService(void){
    void *sbServices = dlopen(kSpringBoardServiceLib, RTLD_LAZY);
    if (!sbServices) {
        /* fail to load the library */
        NSLog(@">>>> [daemonn] dlopen error: %s", dlerror());
        return;
    }
    g_ptrSBSSuspendFrontmostApplication = (void(*)())dlsym(sbServices, "SBSSuspendFrontmostApplication");
    g_ptrSBSCopyFrontmostApplicationDisplayIdentifier = (CFStringRef(*)())dlsym(sbServices, "SBSCopyFrontmostApplicationDisplayIdentifier");
    g_SBSLaunchApplicationWithIdentifier = (int(*)(CFStringRef,Boolean))dlsym(sbServices, "SBSLaunchApplicationWithIdentifier");
    g_ptrSBSProcessIDForDisplayIdentifier = (BOOL(*)(CFStringRef, pid_t*))dlsym(sbServices, "SBSProcessIDForDisplayIdentifier");
    g_ptrSBSOpenSensitiveURLAndUnlock = (int(*)(CFURLRef, char))dlsym(sbServices, "SBSOpenSensitiveURLAndUnlock");
}

void initBiometricKitLib(void);
void initBiometricKitLib(void){
    void *handle = dlopen(kBiometricKitLib, RTLD_LAZY);
    if (!handle) {
        /* fail to load the library */
        NSLog(@">>>> [daemonn] dlopen error: %s", dlerror());
        return;
    }
}

static NSString *udid(void){
    void *handle = dlopen(kAppleAccountLib, RTLD_LAZY);
    if (!handle) {
        NSLog(@">>>> [%s] dlopen error: %s", kAppleAccountLib, dlerror());
    }
    return [NSClassFromString(@"AADeviceInfo") invoke:@"udid"];
}



void sendHomeButtonEvent(void);
uint32_t registerNotification(const char *notification, int *fd, int *token);

/// iOS 7 及以上系统，GSSendSystemEvent、GSCurrentEventTimestamp 不可用
void sendHomeButtonEvent(void) {
    
    NSLog(@">>>> [daemon] %s", __FUNCTION__);
    uint64_t (*GSCurrentEventTimestamp)() = (uint64_t(*)())dlsym(RTLD_SELF, "GSCurrentEventTimestamp");
    void (*GSSendSystemEvent)(const GSEventRecord* record) = (void(*)(const GSEventRecord* record)) dlsym(RTLD_SELF, "GSSendSystemEvent");
    
    if (GSCurrentEventTimestamp && GSSendSystemEvent) {
        NSLog(@">>>> [daemon] %s %p, %p", __FUNCTION__, GSCurrentEventTimestamp, GSSendSystemEvent);
        struct GSEventRecord record;
        memset(&record, 0, sizeof(record));
        record.timestamp = GSCurrentEventTimestamp();
        record.type = kGSEventMenuButtonDown;
        GSSendSystemEvent(&record);
        record.timestamp = GSCurrentEventTimestamp();
        record.type = kGSEventMenuButtonUp;
        GSSendSystemEvent(&record);
    } else {
        NSLog(@">>>> [daemon] %s not found symbol.", __FUNCTION__);
    }
}

uint32_t registerNotification(const char *notification, int *fd, int *token) {
    static char once = 0;
    uint32_t status = notify_register_file_descriptor(notification, fd, once?NOTIFY_REUSE:0, token);
    if (status != NOTIFY_STATUS_OK) {
        fprintf(stderr, "registration failed (%u)\n", status);
    } else {
        fprintf(stdout, "registered %s with token %d\n", notification, *token);
    }
    once = 1;
    return status;
}

static void hoge(GSEventRef event);

static void listenEvent(){
    NSLog(@">>>> [daemonn] %s", __FUNCTION__);
    NSLog(@"GSEventRegisterEventCallBack");
    void (*GSEventRegisterEventCallBack)(void(*callback)(GSEventRef event)) = (void(*)(void(*callback)(GSEventRef event))) dlsym(RTLD_SELF, "GSEventRegisterEventCallBack");
    GSEventRegisterEventCallBack(*hoge);
}

void hoge(GSEventRef event){
    NSLog(@">>>> [daemonn] %s", __FUNCTION__);
}

#pragma mark - CYAppManager

@interface CYAppManager : NSObject

- (int)launchApp:(NSString *)identifier;

- (void)timerFireHandler:(NSTimer *)timer;

@end

@implementation CYAppManager

- (void)timerFireHandler:(NSTimer *)timer{
    NSDictionary *userinfo = timer.userInfo;
    NSString *type = [userinfo objectForKey:@"type"];
    NSString *bundleID = [userinfo objectForKey:@"bundleid"];
    NSDate *date = [[NSDate alloc] init];
    NSLog(@">>>> [daemon]  current datetime: %f, type value: %@", date.timeIntervalSince1970, type);
    
    if ([type isEqualToString:@"launch"]) {
        [self launchApp:bundleID];
    } else if ([type isEqualToString:@"kill"]){
        [self killAppWithIdentifier:bundleID];
    } else if ([type isEqualToString:@"openurl"]){
        NSString *scheme = [timer.userInfo objectForKey:@"url"];
        if (scheme && scheme.length > 0) {
            BOOL ret = [self openURL:scheme];
            NSLog(@">>>> [daemon]  open url: %@, ret: %@", scheme, @(ret));
            if (ret) {
                int t = 30 + rand() % 120;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(t * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    [self killAppWithIdentifier:bundleID];
                });
            }
        }
    }
}


/// sorry, this method doesn't work
- (void)homeButtonClicked{
    Class cls = NSClassFromString(@"BiometricKit");
    
    id manager = [cls invoke:@"manager"];
    NSLog(@">>>> [daemon] %s %@, %@", __FUNCTION__, cls, manager);
    [manager invoke:@"_homeButtonPressed"];
}

- (void)suspendFrontmostApp{
    if (g_ptrSBSSuspendFrontmostApplication) {
        g_ptrSBSSuspendFrontmostApplication();
    }
}

- (NSString *)getFrontmostAppIdentifier{
    if (g_ptrSBSCopyFrontmostApplicationDisplayIdentifier) {
        NSString *identifier = (__bridge NSString *)g_ptrSBSCopyFrontmostApplicationDisplayIdentifier();
        return identifier;
    }
    return nil;
}

- (BOOL)getProcessIDWithIdentifier:(NSString *)bundleID pid:(pid_t *)pid{
    if (g_ptrSBSProcessIDForDisplayIdentifier) {
        return g_ptrSBSProcessIDForDisplayIdentifier((__bridge CFStringRef)bundleID, pid);
    }
    return nil;
}

- (int)killAppWithIdentifier:(NSString *)bundleID{
    NSString *frontmostAppID = [self getFrontmostAppIdentifier];
    NSLog(@">>>> [daemon] %s frontmost app : %@", __FUNCTION__, frontmostAppID);
    if ([bundleID isEqualToString:frontmostAppID] == NO) {
        return  0;
    }
    
    pid_t pid;
    BOOL ret = [self getProcessIDWithIdentifier:bundleID pid:&pid];
    if (ret) {
        ret = kill(pid, SIGQUIT); // SIGQUIT, SIGKILL
        NSLog(@">>>> [daemon] %s frontmost app : %@, pid: %@, kill status: %@", __FUNCTION__, bundleID, @(pid), @(ret));
        return ret;
    } else {
        return -1;
    }
}

- (int)openURL:(NSString *)url{
    if (url && url.length > 0) {
        NSURL *nsurl = [NSURL URLWithString:url];
        return g_ptrSBSOpenSensitiveURLAndUnlock((__bridge CFURLRef)nsurl, 1);
    }
    return -1;
}

- (int)launchApp:(NSString *)bundleID{
    NSString *frontmostAppID = [self getFrontmostAppIdentifier];
    if ([bundleID isEqualToString:frontmostAppID]) {
        
        int flag = rand() % 10;
        if (flag == 3) {
            // first suspend app, then active
            NSLog(@">>>> [daemon] %s suspend app: %@", __FUNCTION__, frontmostAppID);
            [self suspendFrontmostApp];
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                if (g_SBSLaunchApplicationWithIdentifier) {
                    g_SBSLaunchApplicationWithIdentifier((__bridge CFStringRef)bundleID, false);
                }
            });
        }
        
        return  0;
    }

    if (g_SBSLaunchApplicationWithIdentifier) {
        return g_SBSLaunchApplicationWithIdentifier((__bridge CFStringRef)bundleID, false);
    }
    return -1;
}

//-(void)launch:(NSString *)bundle {
//    NSLog(@">>> %s", __FUNCTION__);
//    Class SBApplicationController = objc_getClass("SBApplicationController");
//    NSLog(@">>>> SBApplicationController: %@", SBApplicationController);
//    id appController = [SBApplicationController sharedInstanceIfExists];
//    NSLog(@">>>> appController: %@", appController);
//    id app = [appController applicationWithBundleIdentifier: bundle];
//    if (app) {
//        [self performSelector:@selector(launchTheApp:) withObject:app afterDelay: 0.5];
//    } else {
//        NSLog(@">>>> not found app %@", app);
//    }
//}
//
//-(void)launchTheApp:(id)app {
//    NSLog(@">>> %s", __FUNCTION__);
//    Class SBUIController = objc_getClass("SBUIController");
//    id uiController = [SBUIController sharedInstance];
//    [uiController activateApplication: app];
//}

@end

#pragma mark - main

int main (int argc, const char * argv[]){
    @autoreleasepool {
        initSpringService();
        
        initBiometricKitLib();
        
    //    static int fd;
    //    static int token = 0;
    //    if (registerNotification("com.uroboro.notify.home1", &fd, &token) != NOTIFY_STATUS_OK) {
    //        NSLog(@">>>> [idaemon] register notification failed.");
    //    }
    //
    //    listenEvent();
        
        CYAppManager *manager = [[CYAppManager alloc] init];
            
        NSString *bundleId = @"com.ss.iphone.ugc.Aweme";
        NSTimer *launchAppTimer = [[NSTimer alloc] initWithFireDate:[NSDate date] interval:23.0 target:manager selector:@selector(timerFireHandler:) userInfo:@{@"type": @"launch", @"bundleid": bundleId} repeats:NO];
        
        //sinaweibo://userinfo?uid=<uid>
        //weibointernational://userinfo?uid=<uid>
        //weibointernational://gotohome
        
        NSDictionary *userInfo = @{
            @"type": @"openurl",
            @"url": @"weibointernational://gotohome",
            @"bundleid": @"com.weibo.international",
        };
        NSTimer *openURLTimer = [[NSTimer alloc] initWithFireDate:[NSDate date] interval:300 target:manager selector:@selector(timerFireHandler:) userInfo:userInfo repeats:YES];
        
        NSTimer *killTimer = [[NSTimer alloc] initWithFireDate:[NSDate date] interval:1800 target:manager selector:@selector(timerFireHandler:) userInfo:@{@"type": @"kill", @"bundleid": bundleId} repeats:NO];
        
        NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
        [runLoop addTimer:launchAppTimer forMode:NSDefaultRunLoopMode];
        [runLoop addTimer:openURLTimer forMode:NSDefaultRunLoopMode];
        [runLoop addTimer:killTimer forMode:NSDefaultRunLoopMode];
        [runLoop run];
    }
    return 0;
}

