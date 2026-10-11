#include "AndroidShare.hpp"
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include <Geode/loader/Dirs.hpp>
#include <jni.h>
#include <vector>

namespace fs = std::filesystem;

bool shareMacroFiles(std::vector<fs::path> const& files, std::string& error) {
    error.clear();
    if (files.empty() || files.size() > 30) { error = "Select between 1 and 30 files."; return false; }
    for (auto const& f : files) {
        std::error_code ec;
        if (fs::is_symlink(f, ec) || ec || !fs::is_regular_file(f, ec) || ec || fs::file_size(f, ec) > 64ull * 1024 * 1024 || ec) {
            error = "A selected macro is missing, inaccessible, or too large. Reopen the list.";
            return false;
        }
    }
    auto* vm = cocos2d::JniHelper::getJavaVM();
    JNIEnv* env = nullptr;
    if (!vm || vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK || !env) {
        error = "Android JNI environment unavailable (stage: VM)."; return false;
    }
    std::vector<jobject> refs;
    auto ref = [&](jobject o) -> jobject { if (o) refs.push_back(o); return o; };
    auto klass = [&](char const* name) -> jclass {
        auto c = cocos2d::JniHelper::getClassID(name, env);
        if (c) ref(c);
        return c;
    };
    auto str = [&](std::string const& text) -> jstring {
        return static_cast<jstring>(ref(env->NewStringUTF(text.c_str())));
    };
    auto clear = [&] { if (env->ExceptionCheck()) env->ExceptionClear(); };
    std::string stage = "activity";
    bool success = false;
    do {
        jobject activity = nullptr;
        auto launcher = klass("com/customRobTop/BaseRobTopActivity");
        if (launcher) {
            auto field = env->GetStaticFieldID(launcher, "INSTANCE", "Lcom/customRobTop/BaseRobTopActivity;");
            if (field) {
                auto inst = ref(env->GetStaticObjectField(launcher, field));
                auto getMe = env->GetMethodID(launcher, "getMe", "()Ljava/lang/ref/WeakReference;");
                if (inst && getMe) {
                    auto weak = ref(env->CallObjectMethod(inst, getMe));
                    auto weakClass = klass("java/lang/ref/WeakReference");
                    if (weak && weakClass) {
                        auto get = env->GetMethodID(weakClass, "get", "()Ljava/lang/Object;");
                        if (get) activity = ref(env->CallObjectMethod(weak, get));
                    }
                }
            }
        }
        if (!activity) {
            clear();
            // Older game/launcher combinations expose Cocos2dxActivity.getContext.
            auto cocos = klass("org/cocos2dx/lib/Cocos2dxActivity");
            if (cocos) {
                auto method = env->GetStaticMethodID(cocos, "getContext", "()Landroid/content/Context;");
                if (method) activity = ref(env->CallStaticObjectMethod(cocos, method));
            }
        }
        if (!activity || env->ExceptionCheck()) break;
        stage = "package name";
        auto contextClass = static_cast<jclass>(ref(env->GetObjectClass(activity)));
        if (!contextClass) break;
        auto getPackageName = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
        if (!getPackageName) break;
        auto package = static_cast<jstring>(ref(env->CallObjectMethod(activity, getPackageName)));
        if (!package || env->ExceptionCheck()) break;
        auto chars = env->GetStringUTFChars(package, nullptr);
        if (!chars) break;
        auto packageName = std::string(chars);
        env->ReleaseStringUTFChars(package, chars);
        stage = "content URI";
        std::vector<jobject> uris;
        auto fileClass = klass("java/io/File");
        auto provider = klass("androidx/core/content/FileProvider");
        jmethodID fileCtor = fileClass ? env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V") : nullptr;
        jmethodID providerMethod = provider ? env->GetStaticMethodID(provider, "getUriForFile",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/io/File;)Landroid/net/Uri;") : nullptr;
        clear(); // Missing FileProvider on older launchers is handled by fallback.
        jclass documents = nullptr;
        jmethodID buildDocumentUri = nullptr;
        for (auto const& path : files) {
            jobject uri = nullptr;
            if (fileCtor && providerMethod) {
                auto jFile = ref(env->NewObject(fileClass, fileCtor, str(path.string())));
                if (jFile && !env->ExceptionCheck()) {
                    uri = env->CallStaticObjectMethod(provider, providerMethod, activity,
                        str(packageName + ".fileprovider"), jFile);
                    if (env->ExceptionCheck()) { clear(); uri = nullptr; }
                }
            }
            if (!uri) {
                clear();
                // The standard launcher also exposes a DocumentsProvider for
                // files inside its media directory, including profile folders.
                auto base = geode::dirs::getGameDir().parent_path();
                if (base.parent_path().filename() == "profiles") base = base.parent_path().parent_path();
                std::error_code ec;
                auto relative = fs::weakly_canonical(path, ec).lexically_relative(fs::weakly_canonical(base, ec));
                if (ec || relative.empty() || relative.is_absolute()) break;
                bool outside = false;
                for (auto const& segment : relative) if (segment == "..") outside = true;
                if (outside) break;
                if (!documents) {
                    documents = klass("android/provider/DocumentsContract");
                    if (!documents) break;
                    buildDocumentUri = env->GetStaticMethodID(documents, "buildDocumentUri",
                        "(Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;");
                    if (!buildDocumentUri) break;
                }
                uri = env->CallStaticObjectMethod(documents, buildDocumentUri,
                    str(packageName + ".user"), str("root" + relative.generic_string()));
                if (env->ExceptionCheck()) break;
            }
            if (!uri) break;
            uris.push_back(ref(uri));
        }
        if (uris.size() != files.size() || env->ExceptionCheck()) break;
        stage = "Intent setup";
        auto intentClass = klass("android/content/Intent");
        if (!intentClass) break;
        auto ctor = env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;)V");
        if (!ctor) break;
        bool multiple = files.size() > 1;
        auto intent = ref(env->NewObject(intentClass, ctor,
            str(multiple ? "android.intent.action.SEND_MULTIPLE" : "android.intent.action.SEND")));
        if (!intent || env->ExceptionCheck()) break;
        auto setType = env->GetMethodID(intentClass, "setType", "(Ljava/lang/String;)Landroid/content/Intent;");
        auto addFlags = env->GetMethodID(intentClass, "addFlags", "(I)Landroid/content/Intent;");
        if (!setType || !addFlags) break;
        stage = "attach file URI";
        if (multiple) {
            auto listClass = klass("java/util/ArrayList");
            if (!listClass) break;
            auto newList = env->GetMethodID(listClass, "<init>", "()V");
            auto add = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");
            auto putList = env->GetMethodID(intentClass, "putParcelableArrayListExtra",
                "(Ljava/lang/String;Ljava/util/ArrayList;)Landroid/content/Intent;");
            if (!newList || !add || !putList) break;
            auto list = ref(env->NewObject(listClass, newList));
            if (!list) break;
            for (auto uri : uris) env->CallBooleanMethod(list, add, uri);
            auto ret = env->CallObjectMethod(intent, putList, str("android.intent.extra.STREAM"), list);
            if (ret) env->DeleteLocalRef(ret);
        } else {
            auto put = env->GetMethodID(intentClass, "putExtra",
                "(Ljava/lang/String;Landroid/os/Parcelable;)Landroid/content/Intent;");
            if (!put) break;
            auto ret = env->CallObjectMethod(intent, put, str("android.intent.extra.STREAM"), uris[0]);
            if (ret) env->DeleteLocalRef(ret);
        }
        if (env->ExceptionCheck()) break;
        stage = "set MIME and grant";
        auto ret = env->CallObjectMethod(intent, setType, str("application/octet-stream"));
        if (ret) env->DeleteLocalRef(ret);
        if (env->ExceptionCheck()) break;
        ret = env->CallObjectMethod(intent, addFlags, 1); // FLAG_GRANT_READ_URI_PERMISSION
        if (ret) env->DeleteLocalRef(ret);
        if (env->ExceptionCheck()) break;
        stage = "ClipData";
        auto clipClass = klass("android/content/ClipData");
        if (!clipClass) break;
        auto newRawUri = env->GetStaticMethodID(clipClass, "newRawUri",
            "(Ljava/lang/CharSequence;Landroid/net/Uri;)Landroid/content/ClipData;");
        auto clip = newRawUri ? ref(env->CallStaticObjectMethod(clipClass, newRawUri, str("xdBot macros"), uris[0])) : nullptr;
        if (!clip || env->ExceptionCheck()) break;
        if (multiple) {
            auto itemClass = klass("android/content/ClipData$Item");
            auto itemCtor = itemClass ? env->GetMethodID(itemClass, "<init>", "(Landroid/net/Uri;)V") : nullptr;
            auto addItem = env->GetMethodID(clipClass, "addItem", "(Landroid/content/ClipData$Item;)V");
            if (!itemCtor || !addItem) break;
            for (std::size_t i = 1; i < uris.size(); ++i) {
                auto item = ref(env->NewObject(itemClass, itemCtor, uris[i]));
                if (!item) break;
                env->CallVoidMethod(clip, addItem, item);
            }
        }
        // Android Intent.setClipData returns void, unlike fluent setType/addFlags.
        // Looking it up as a method returning Intent caused the send-intent error.
        stage = "Intent.setClipData";
        auto setClip = env->GetMethodID(intentClass, "setClipData", "(Landroid/content/ClipData;)V");
        if (!setClip || env->ExceptionCheck()) break;
        env->CallVoidMethod(intent, setClip, clip);
        if (env->ExceptionCheck()) break;
        stage = "create chooser";
        auto createChooser = env->GetStaticMethodID(intentClass, "createChooser",
            "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;");
        if (!createChooser || env->ExceptionCheck()) break;
        auto chooser = ref(env->CallStaticObjectMethod(intentClass, createChooser, intent, str("Share saved macros")));
        if (!chooser || env->ExceptionCheck()) break;
        ret = env->CallObjectMethod(chooser, addFlags, 1);
        if (ret) env->DeleteLocalRef(ret);
        stage = "launch chooser";
        auto start = env->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");
        if (!start) break;
        env->CallVoidMethod(activity, start, chooser);
        if (env->ExceptionCheck()) break;
        success = true;
    } while (false);
    clear();
    for (auto it = refs.rbegin(); it != refs.rend(); ++it) env->DeleteLocalRef(*it);
    if (!success) error = "Could not share from the Geode launcher (stage: " + stage + "). Send this stage and your launcher version so it can be diagnosed.";
    return success;
}
