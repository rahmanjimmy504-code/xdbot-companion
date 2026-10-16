#include "HyperbolusAndroid.hpp"
#include "MacroExport.hpp"
#include <Geode/Geode.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include <jni.h>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
namespace {
jclass s_bridge = nullptr; // Global JNI reference: keep the dynamically loaded dex alive.
jobject s_classLoader = nullptr;
}

static bool invokeAndroidDocumentBridge(std::optional<fs::path> selected, char const* method, std::string& error) {
    if (selected && !validateMacroForExport(*selected, error)) return false;
    auto resourceDex = geode::Mod::get()->getResourcesDir() / "hyperbolus-bridge.dex";
    std::error_code ec;
    if (!fs::is_regular_file(resourceDex, ec) || ec) {
        error = "The embedded Android browser was not packaged. No website was opened.";
        return false;
    }
    auto saveDir = geode::Mod::get()->getSaveDir();
    auto dex = saveDir / "hyperbolus-bridge-v1.3.0-beta.9.dex";
    if (!s_bridge) {
        fs::create_directories(saveDir, ec);
        if (!ec) fs::remove(dex, ec);
        if (!ec) fs::copy_file(resourceDex, dex, fs::copy_options::none, ec);
        // Android 14+ refuses to load writable dex files. Leave the Geode
        // resource untouched, and make only our private staging copy read-only.
        if (!ec) fs::permissions(dex, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read,
            fs::perm_options::replace, ec);
        if (ec) {
            error = "Could not stage the embedded Android browser safely.";
            return false;
        }
    }
    auto* vm = cocos2d::JniHelper::getJavaVM();
    JNIEnv* env = nullptr;
    if (!vm || vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK || !env) {
        error = "Android JNI environment unavailable."; return false;
    }
    std::vector<jobject> locals;
    auto keep = [&](jobject obj) -> jobject { if (obj) locals.push_back(obj); return obj; };
    auto klass = [&](char const* name) -> jclass {
        return static_cast<jclass>(keep(cocos2d::JniHelper::getClassID(name, env)));
    };
    auto str = [&](std::string const& text) -> jstring {
        return static_cast<jstring>(keep(env->NewStringUTF(text.c_str())));
    };
    auto clear = [&] { if (env->ExceptionCheck()) { env->ExceptionClear(); return true; } return false; };
    std::string stage = "activity";
    bool success = false;
    do {
        jobject activity = nullptr;
        auto launcher = klass("com/customRobTop/BaseRobTopActivity");
        if (launcher) {
            auto field = env->GetStaticFieldID(launcher, "INSTANCE", "Lcom/customRobTop/BaseRobTopActivity;");
            if (field) {
                auto instance = keep(env->GetStaticObjectField(launcher, field));
                auto getMe = env->GetMethodID(launcher, "getMe", "()Ljava/lang/ref/WeakReference;");
                if (instance && getMe) {
                    auto weak = keep(env->CallObjectMethod(instance, getMe));
                    auto weakClass = klass("java/lang/ref/WeakReference");
                    auto get = weakClass ? env->GetMethodID(weakClass, "get", "()Ljava/lang/Object;") : nullptr;
                    if (weak && get) activity = keep(env->CallObjectMethod(weak, get));
                }
            }
        }
        if (!activity) {
            clear();
            auto cocos = klass("org/cocos2dx/lib/Cocos2dxActivity");
            auto method = cocos ? env->GetStaticMethodID(cocos, "getContext", "()Landroid/content/Context;") : nullptr;
            if (method) activity = keep(env->CallStaticObjectMethod(cocos, method));
        }
        if (!activity || env->ExceptionCheck()) break;
        auto activityClass = static_cast<jclass>(keep(env->GetObjectClass(activity)));
        if (!activityClass) break;
        auto getLoader = env->GetMethodID(activityClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
        auto getPackage = env->GetMethodID(activityClass, "getPackageName", "()Ljava/lang/String;");
        if (!getLoader || !getPackage) break;
        auto parent = keep(env->CallObjectMethod(activity, getLoader));
        auto package = static_cast<jstring>(keep(env->CallObjectMethod(activity, getPackage)));
        if (!parent || !package || env->ExceptionCheck()) break;
        auto chars = env->GetStringUTFChars(package, nullptr);
        if (!chars) break;
        std::string packageName(chars);
        env->ReleaseStringUTFChars(package, chars);
        stage = "selected macro URI";
        jobject macroUri = nullptr;
        if (selected) {
            auto file = klass("java/io/File");
            auto provider = klass("androidx/core/content/FileProvider");
            auto fileCtor = file ? env->GetMethodID(file, "<init>", "(Ljava/lang/String;)V") : nullptr;
            auto uriMethod = provider ? env->GetStaticMethodID(provider, "getUriForFile",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/io/File;)Landroid/net/Uri;") : nullptr;
            if (fileCtor && uriMethod) {
                auto jfile = keep(env->NewObject(file, fileCtor, str(selected->string())));
                if (jfile && !env->ExceptionCheck())
                    macroUri = keep(env->CallStaticObjectMethod(provider, uriMethod, activity,
                        str(packageName + ".fileprovider"), jfile));
            }
            if (clear() || !macroUri) {
                auto base = geode::dirs::getGameDir().parent_path();
                if (base.parent_path().filename() == "profiles") base = base.parent_path().parent_path();
                std::error_code pathError;
                auto selectedReal = fs::weakly_canonical(*selected, pathError);
                auto baseReal = fs::weakly_canonical(base, pathError);
                auto relative = selectedReal.lexically_relative(baseReal);
                if (pathError || relative.empty() || relative.is_absolute()) break;
                bool outside = false;
                for (auto const& segment : relative) if (segment == "..") outside = true;
                if (outside) break;
                auto docs = klass("android/provider/DocumentsContract");
                auto build = docs ? env->GetStaticMethodID(docs, "buildDocumentUri",
                    "(Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;") : nullptr;
                if (!build) break;
                macroUri = keep(env->CallStaticObjectMethod(docs, build,
                    str(packageName + ".user"), str("root" + relative.generic_string())));
            }
            if (!macroUri || env->ExceptionCheck()) break;
        }
        stage = "load embedded browser";
        if (!s_bridge) {
            auto dexClass = klass("dalvik/system/DexClassLoader");
            auto ctor = dexClass ? env->GetMethodID(dexClass, "<init>",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V") : nullptr;
            if (!ctor) break;
            auto loaded = keep(env->NewObject(dexClass, ctor, str(dex.string()),
                str(geode::Mod::get()->getSaveDir().string()), nullptr, parent));
            if (!loaded || env->ExceptionCheck()) break;
            auto loaderClass = static_cast<jclass>(keep(env->GetObjectClass(loaded)));
            auto load = env->GetMethodID(loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
            if (!load) break;
            auto result = keep(env->CallObjectMethod(loaded, load,
                str("com.jimmy1234ha.xdbot.HyperbolusBridge")));
            if (!result || env->ExceptionCheck()) break;
            s_bridge = static_cast<jclass>(env->NewGlobalRef(result));
            s_classLoader = env->NewGlobalRef(loaded);
            if (!s_bridge || !s_classLoader) break;
        }
        stage = "invoke native document UI";
        auto call = env->GetStaticMethodID(s_bridge, method,
            "(Landroid/app/Activity;Landroid/net/Uri;Ljava/lang/String;)V");
        if (!call) break;
        auto name = str(selected ? selected->filename().string() : "");
        env->CallStaticVoidMethod(s_bridge, call, activity, macroUri, name);
        if (env->ExceptionCheck()) break;
        success = true;
    } while (false);
    clear();
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) env->DeleteLocalRef(*it);
    if (!success) error = "Android document bridge could not start (stage: " + stage + ").";
    return success;
}

bool openHyperbolusAndroid(std::optional<fs::path> selected, std::string& error) {
    return invokeAndroidDocumentBridge(std::move(selected), "open", error);
}

bool exportMacroAndroid(fs::path const& source, std::string& error) {
    return invokeAndroidDocumentBridge(source, "export", error);
}

bool openGDBrowserAndroid(std::string& error) {
    return invokeAndroidDocumentBridge(std::nullopt, "openGDB", error);
}
