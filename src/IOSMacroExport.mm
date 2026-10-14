#include "MacroExport.hpp"
#include <Geode/Geode.hpp>
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

// UIKit exports an existing file to a chosen Files location. Unlike the
// Geode SaveFile picker in SDK 5.10.1, this does not create/truncate an empty
// staging file before presenting the document export picker.
@interface CompanionExportDelegate : NSObject <UIDocumentPickerDelegate>
@end

namespace {
CompanionExportDelegate* g_delegate = nil;
bool g_pickerOpen = false;

void notice(std::string const& title, std::string const& message) {
    geode::queueInMainThread([title, message] {
        geode::createQuickPopup(title.c_str(), message, "OK", nullptr, [](auto*, bool) {});
    });
}
}

@implementation CompanionExportDelegate
- (void)documentPicker:(UIDocumentPickerViewController*)controller
    didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls {
    CompanionExportDelegate* keepAlive = self;
    g_pickerOpen = false;
    g_delegate = nil;
    if (urls.count > 0) notice("Macro exported", "A copy was saved to your chosen location.");
    (void)keepAlive;
    else notice("Export unavailable", "The selected destination could not be confirmed.");
}
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController*)controller {
    CompanionExportDelegate* keepAlive = self;
    g_pickerOpen = false;
    g_delegate = nil;
    (void)keepAlive;
}
@end

void exportMacroFile(std::filesystem::path const& source) {
    std::string error;
    if (!validateMacroForExport(source, error)) {
        notice("Export unavailable", error);
        return;
    }
    auto native = source.string();
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_pickerOpen) {
            notice("Export unavailable", "Finish the current file picker first.");
            return;
        }
        NSString* path = [NSString stringWithUTF8String:native.c_str()];
        if (!path) {
            notice("Export unavailable", "The macro path could not be decoded.");
            return;
        }
        NSURL* url = [NSURL fileURLWithPath:path isDirectory:NO];
        if (!url) {
            notice("Export unavailable", "The selected macro could not be opened.");
            return;
        }
        UIApplication* app = [UIApplication sharedApplication];
        UIWindow* window = app.keyWindow ?: app.windows.firstObject;
        UIViewController* controller = window.rootViewController;
        while (controller.presentedViewController) controller = controller.presentedViewController;
        if (!controller) {
            notice("Export unavailable", "Could not find the game window.");
            return;
        }
        UIDocumentPickerViewController* picker = [[UIDocumentPickerViewController alloc]
            initForExportingURLs:@[url] asCopy:YES];
        g_delegate = [CompanionExportDelegate new];
        g_pickerOpen = true;
        picker.delegate = g_delegate;
        [controller presentViewController:picker animated:YES completion:nil];
    });
}
