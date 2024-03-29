//
//  SBApplicationController.h
//  idaemond
//
//  Created by jiaxw on 2020/11/16.
//

#ifndef SBApplicationController_h
#define SBApplicationController_h

@interface SBApplicationController : NSObject
+(id)sharedInstance;
+(id)sharedInstanceIfExists;
-(void)waitForUninstallsToComplete;
-(id)restrictionController;
-(void)_mediaServerConnectionDied:(id)arg1 ;
-(id)setupApplication;
-(id)runningApplications;
-(id)musicApplication;
-(id)dataActivationApplication;
-(id)faceTimeApp;
-(id)cameraApplication;
-(void)requestUninstallApplication:(id)arg1 withCompletion:(/*^block*/id)arg2 ;
-(id)testFlightApplication;
-(id)webApplications;
-(id)_loadApplications:(id)arg1 removed:(id)arg2 ;
-(void)_lockStateChanged:(id)arg1 ;
-(void)_memoryWarningReceived;
-(void)_finishDeferredMajorVersionMigrationTasks_FlushSystemSnapshots;
-(void)_finishDeferredMajorVersionMigrationTasks_FlushLegacySnapshots;
-(id)_lock_applicationWithBundleIdentifier:(id)arg1 ;
-(void)_removeApplicationsFromModelWithBundleIdentifier:(id)arg1 forInstall:(BOOL)arg2 withReason:(id)arg3 ;
-(void)_sendInstalledAppsDidChangeNotification:(id)arg1 removed:(id)arg2 modified:(id)arg3 ;
-(void)_unusuallyMutedAudioPlaying:(id)arg1 ;
-(Class)_appClassForInfo:(id)arg1 ;
-(void)_preLoadApplications;
-(id)_loadApplicationFromApplicationInfo:(id)arg1 ;
-(id)_appInfosToBundleIDs:(id)arg1 ;
-(void)_updateIconControllerAndModelForLoadedApplications:(id)arg1 reveal:(BOOL)arg2 popIn:(BOOL)arg3 reloadAllIcons:(BOOL)arg4 ;
-(void)_loadApplicationsAndIcons:(id)arg1 removed:(id)arg2 reveal:(BOOL)arg3 ;
-(void)_updateVisibilityOverrides;
-(id)allBundleIdentifiers;
-(id)alwaysAvailableApplicationBundleIdentifiers;
-(id)bundleIdentifiersWithVisibilityOverrideHidden;
-(void)applicationsAdded:(id)arg1 ;
-(void)applicationsModified:(id)arg1 ;
-(void)applicationsRemoved:(id)arg1 ;
-(void)applicationsDemoted:(id)arg1 ;
-(void)applicationVisibilityMayHaveChanged;
-(void)applicationRestrictionsMayHaveChanged;
-(BOOL)scheduler:(id)arg1 requestsApplicationLaunch:(id)arg2 ;
-(void)scheduler:(id)arg1 didWakeApplication:(id)arg2 ;
-(id)applicationToLaunchWithLeafIdentifier:(id)arg1 ;
-(id)inCallServiceApp;
-(id)clockApplication;
-(id)iPodOutApplication;
-(id)mapsApplication;
-(id)loginApplication;
-(id)notesApplication;
-(BOOL)_loadApplicationWithoutMutatingIconState:(id)arg1 ;
-(id)allApplications;
-(void)applicationService:(id)arg1 setBadgeValue:(id)arg2 forBundleIdentifier:(id)arg3 ;
-(void)applicationService:(id)arg1 getBadgeValueForBundleIdentifier:(id)arg2 withCompletion:(/*^block*/id)arg3 ;
-(void)applicationService:(id)arg1 deleteAllSnapshotsForBundleIdentifier:(id)arg2 ;
-(void)applicationService:(id)arg1 didSetBrightness:(double)arg2 forBundleIdentifier:(id)arg3 ;
-(void)applicationService:(id)arg1 setNextWakeDate:(id)arg2 forBundleIdentifier:(id)arg3 ;
-(void)applicationService:(id)arg1 suspendApplicationWithBundleIdentifier:(id)arg2 ;
-(void)noteTerminationAssertionEfficacyChangedTo:(unsigned long long)arg1 forBundleIdentifier:(id)arg2 ;
-(id)mobilePhone;
-(void)_unregisterForAVSystemControllerNotifications;
-(void)_registerForAVSystemControllerNotifications;
-(id)applicationWithPid:(int)arg1 ;
-(id)applicationWithBundleIdentifier:(id)arg1 ;
-(void)uninstallApplication:(id)arg1 ;
@end


#endif /* SBApplicationController_h */
