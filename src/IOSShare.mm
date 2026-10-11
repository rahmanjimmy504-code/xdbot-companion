#include "SharePlatform.hpp"
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

// iOS developer preview: a UIKit share-sheet implementation. It still needs
// an iOS build, sandbox access checks and a device test with xdBot.
bool shareMacroFilesPlatform(std::vector<std::filesystem::path> const& files, std::string& error) {
    @autoreleasepool {
        NSMutableArray<NSURL*>* urls = [NSMutableArray arrayWithCapacity:files.size()];
        for (auto const& path : files) {
            auto native = path.string();
            NSString* string = [NSString stringWithUTF8String:native.c_str()];
            if (!string) { error = "An iOS macro path could not be decoded."; return false; }
            NSURL* url = [NSURL fileURLWithPath:string isDirectory:NO];
            if (!url) { error = "An iOS macro file URL could not be created."; return false; }
            [urls addObject:url];
        }
        NSArray<NSURL*>* items = [urls copy];
        __block BOOL opened = NO;
        void (^presentSheet)(void) = ^{
            UIApplication* app = [UIApplication sharedApplication];
            UIWindow* window = app.keyWindow;
            if (!window) {
                for (UIWindow* candidate in app.windows) {
                    if (candidate.isKeyWindow) { window = candidate; break; }
                }
            }
            if (!window) window = app.windows.firstObject;
            UIViewController* controller = window.rootViewController;
            while (controller.presentedViewController) controller = controller.presentedViewController;
            if (!controller || !controller.view) return;
            UIActivityViewController* sheet = [[UIActivityViewController alloc]
                initWithActivityItems:items applicationActivities:nil];
            UIPopoverPresentationController* popover = sheet.popoverPresentationController;
            if (popover) {
                popover.sourceView = controller.view;
                popover.sourceRect = CGRectMake(CGRectGetMidX(controller.view.bounds),
                    CGRectGetMidY(controller.view.bounds), 1, 1);
                popover.permittedArrowDirections = 0;
            }
            [controller presentViewController:sheet animated:YES completion:nil];
            opened = YES;
        };
        if ([NSThread isMainThread]) presentSheet();
        else dispatch_sync(dispatch_get_main_queue(), presentSheet);
        if (!opened) error = "Could not find the iOS game window for sharing.";
        return opened;
    }
}
