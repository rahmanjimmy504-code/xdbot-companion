package com.jimmy1234ha.xdbot;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.OpenableColumns;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.CookieManager;
import android.webkit.DownloadListener;
import android.webkit.SslErrorHandler;
import android.net.http.SslError;
import android.webkit.URLUtil;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.net.HttpURLConnection;
import java.net.URL;

/** Native Android WebView over Geometry Dash. No password or session is passed to native mod code. */
public final class HyperbolusBridge extends WebChromeClient implements DownloadListener {
    private static HyperbolusBridge active;
    private final Activity activity;
    private final Uri selectedMacro;
    private final String selectedName;
    private WebView web;
    private AlertDialog dialog;
    private Object fileLauncher;
    private Object saveLauncher;
    private ValueCallback<Uri[]> fileCallback;
    private String pendingDownload;
    private String pendingUserAgent;

    private HyperbolusBridge(Activity activity, Uri selectedMacro, String selectedName) {
        this.activity = activity;
        this.selectedMacro = selectedMacro;
        this.selectedName = selectedName;
    }

    public static void open(Activity activity, Uri macro, String name) {
        activity.runOnUiThread(() -> {
            if (active != null) {
                if (active.dialog != null) active.dialog.show();
                return;
            }
            HyperbolusBridge bridge = new HyperbolusBridge(activity, macro, name);
            if (!bridge.registerPickers()) {
                Toast.makeText(activity, "This Geode launcher cannot open the secure file pickers.", Toast.LENGTH_LONG).show();
                return;
            }
            active = bridge;
            bridge.show();
        });
    }

    private static boolean site(Uri uri) {
        return uri != null && HyperbolusRules.trustedUrl(uri.getScheme(), uri.getHost(), uri.getPort());
    }
    private void toast(String message) { Toast.makeText(activity, message, Toast.LENGTH_LONG).show(); }

    // The Geode Android launcher is an AppCompatActivity. Use its ActivityResultRegistry
    // via reflection so this dex needs only android.jar at compilation, not an AndroidX
    // AAR copied from a possibly different launcher version.
    private Object register(String key, String contractClassName, Class<?>[] ctorTypes, Object[] ctorArgs,
                            InvocationHandler callback) throws Exception {
        ClassLoader loader = activity.getClassLoader();
        Class<?> contractClass = Class.forName(contractClassName, true, loader);
        Object contract = contractClass.getConstructor(ctorTypes).newInstance(ctorArgs);
        Class<?> callbackClass = Class.forName("androidx.activity.result.ActivityResultCallback", true, loader);
        Object proxy = Proxy.newProxyInstance(loader, new Class<?>[]{callbackClass}, callback);
        Object registry = activity.getClass().getMethod("getActivityResultRegistry").invoke(activity);
        Class<?> baseContract = Class.forName("androidx.activity.result.contract.ActivityResultContract", true, loader);
        return registry.getClass().getMethod("register", String.class, baseContract, callbackClass)
            .invoke(registry, key, contract, proxy);
    }
    private boolean registerPickers() {
        try {
            String suffix = "xdbot-companion-hyperbolus";
            // These two contracts are already used by Geode's launcher, so
            // Android's shrinker cannot remove them as unused classes.
            fileLauncher = register(suffix + "-open", "com.geode.launcher.activityresult.GeodeOpenFileActivityResult",
                new Class<?>[0], new Object[0], (proxy, method, args) -> {
                    if (method.getName().equals("onActivityResult")) uploadPicked(args != null ? (Uri) args[0] : null);
                    return null;
                });
            saveLauncher = register(suffix + "-save", "com.geode.launcher.activityresult.GeodeSaveFileActivityResult",
                new Class<?>[0], new Object[0], (proxy, method, args) -> {
                    if (method.getName().equals("onActivityResult")) downloadPicked(args != null ? (Uri) args[0] : null);
                    return null;
                });
            return true;
        } catch (Exception e) {
            unregister(fileLauncher); unregister(saveLauncher);
            fileLauncher = null; saveLauncher = null;
            return false;
        }
    }
    private Object openParams() throws Exception {
        Class<?> cls = Class.forName("com.geode.launcher.activityresult.GeodeOpenFileActivityResult$OpenFileParams",
            true, activity.getClassLoader());
        return cls.getConstructor(String[].class, Uri.class).newInstance((Object)new String[]{"*/*"}, null);
    }
    private Object saveParams(String name) throws Exception {
        Class<?> cls = Class.forName("com.geode.launcher.activityresult.GeodeSaveFileActivityResult$SaveFileParams",
            true, activity.getClassLoader());
        return cls.getConstructor(String.class, File.class).newInstance("application/octet-stream", new File(name));
    }
    private void launch(Object launcher, Object input) {
        try {
            Class<?> clazz = Class.forName("androidx.activity.result.ActivityResultLauncher", true, activity.getClassLoader());
            clazz.getMethod("launch", Object.class).invoke(launcher, input);
        } catch (Exception e) {
            toast("Could not open the Android document picker.");
            if (launcher == fileLauncher) uploadPicked(null);
            else { pendingDownload = null; pendingUserAgent = null; }
        }
    }
    private void unregister(Object launcher) {
        if (launcher == null) return;
        try { launcher.getClass().getMethod("unregister").invoke(launcher); } catch (Exception ignored) { }
    }

    private void show() {
        web = new WebView(activity);
        WebSettings settings = web.getSettings();
        settings.setJavaScriptEnabled(true); // Hyperbolus's own site needs JavaScript.
        settings.setDomStorageEnabled(true);
        settings.setAllowFileAccess(false);
        settings.setAllowContentAccess(true); // Needed for user-selected SAF uploads.
        settings.setJavaScriptCanOpenWindowsAutomatically(false);
        settings.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);
        CookieManager.getInstance().setAcceptThirdPartyCookies(web, false);
        settings.setAllowFileAccessFromFileURLs(false);
        settings.setAllowUniversalAccessFromFileURLs(false);
        if (Build.VERSION.SDK_INT >= 26) settings.setSafeBrowsingEnabled(true);
        web.setWebChromeClient(this);
        web.setDownloadListener(this);
        web.setWebViewClient(new WebViewClient() {
            @Override public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                if (request.isForMainFrame()) {
                    if (!site(request.getUrl())) {
                        toast("Blocked a link outside hyperbolus.net."); return true;
                    }
                    // Capture the site's download link before it redirects to signed
                    // external object storage. The app never forwards site cookies
                    // to the storage host; the user chooses the SAF destination.
                    if (request.getUrl().getPath() != null && request.getUrl().getPath().startsWith("/download/")) {
                        queueDownload(request.getUrl().toString(), view.getSettings().getUserAgentString(),
                            "hyperbolus-macro");
                        return true;
                    }
                }
                return false;
            }
            @Override public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                handler.cancel(); toast("Hyperbolus TLS certificate check failed.");
            }
        });
        LinearLayout content = new LinearLayout(activity);
        content.setOrientation(LinearLayout.VERTICAL);
        LinearLayout nav = new LinearLayout(activity);
        nav.setOrientation(LinearLayout.HORIZONTAL);
        addNav(nav, "LOGIN", "https://hyperbolus.net/auth/login");
        addNav(nav, "REPLAYS", "https://hyperbolus.net/replays");
        addNav(nav, "UPLOAD", "https://hyperbolus.net/replays/new");
        Button back = new Button(activity);
        back.setText("BACK");
        back.setOnClickListener(v -> { if (web.canGoBack()) web.goBack(); });
        nav.addView(back, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        content.addView(nav);
        int height = (int)(activity.getResources().getDisplayMetrics().heightPixels * .70f);
        content.addView(web, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, height));
        dialog = new AlertDialog.Builder(activity).setTitle("Hyperbolus: manual website")
            .setView(content).setNegativeButton("CLOSE", (d, which) -> { }).create();
        dialog.setOnDismissListener(d -> close());
        dialog.show();
        if (dialog.getWindow() != null)
            dialog.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        web.loadUrl("https://hyperbolus.net/auth/login");
    }
    private void addNav(LinearLayout row, String label, String url) {
        Button button = new Button(activity);
        button.setText(label);
        button.setOnClickListener(v -> web.loadUrl(url));
        row.addView(button, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
    }
    private void close() {
        if (fileCallback != null) { fileCallback.onReceiveValue(null); fileCallback = null; }
        unregister(fileLauncher); unregister(saveLauncher);
        if (web != null) { web.stopLoading(); web.destroy(); web = null; }
        dialog = null;
        active = null;
        // Do not erase Hyperbolus's own login cookies; the user can sign out on its website.
    }

    private static boolean supportedUpload(String filename) {
        return HyperbolusRules.supportedUpload(filename);
    }
    private String documentName(Uri uri) {
        try (Cursor cursor = activity.getContentResolver().query(uri, new String[]{OpenableColumns.DISPLAY_NAME}, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) return cursor.getString(0);
        } catch (Exception ignored) { }
        return null;
    }
    private void uploadPicked(Uri uri) {
        if (fileCallback == null) return;
        if (uri != null && !supportedUpload(documentName(uri))) {
            toast("Unsupported for Hyperbolus: choose a .gdr or .json file. .cml and .gdr2 are not accepted.");
            uri = null;
        }
        ValueCallback<Uri[]> callback = fileCallback;
        fileCallback = null;
        callback.onReceiveValue(uri == null ? null : new Uri[]{uri});
    }
    @Override public boolean onShowFileChooser(WebView view, ValueCallback<Uri[]> callback, FileChooserParams params) {
        Uri page = Uri.parse(view.getUrl() == null ? "" : view.getUrl());
        if (!site(page) || !"/replays/new".equals(page.getPath())) {
            toast("Choose files only on Hyperbolus's replay upload page.");
            callback.onReceiveValue(null);
            return true;
        }
        if (fileCallback != null) fileCallback.onReceiveValue(null);
        fileCallback = callback;
        if (selectedMacro != null) {
            if (!supportedUpload(selectedName)) {
                toast("Unsupported for Hyperbolus: .cml and .gdr2 cannot be uploaded.");
                uploadPicked(null); return true;
            }
            new AlertDialog.Builder(activity).setTitle("Attach selected macro?")
                .setMessage(selectedName + "\nNothing uploads yet. To upload, you must also submit Hyperbolus's own form.")
                .setPositiveButton("ATTACH", (d, which) -> uploadPicked(selectedMacro))
                .setNegativeButton("CANCEL", (d, which) -> uploadPicked(null))
                .setOnCancelListener(d -> uploadPicked(null)).show();
        } else {
            try { launch(fileLauncher, openParams()); }
            catch (Exception e) { toast("Launcher file picker is unavailable."); uploadPicked(null); }
        }
        return true;
    }

    @Override public void onDownloadStart(String url, String userAgent, String disposition, String mime, long length) {
        // Downloads must originate on Hyperbolus; storage redirects are followed below without forwarding cookies.
        if (!site(Uri.parse(url))) { toast("Blocked a download outside hyperbolus.net."); return; }
        String name = HyperbolusRules.cleanFilename(URLUtil.guessFileName(url, disposition, mime));
        queueDownload(url, userAgent, name);
    }
    private void queueDownload(String url, String userAgent, String name) {
        if (pendingDownload != null) {
            toast("Finish the current download picker first."); return;
        }
        pendingDownload = url;
        pendingUserAgent = userAgent;
        try { launch(saveLauncher, saveParams(name)); }
        catch (Exception e) {
            pendingDownload = null; pendingUserAgent = null;
            toast("Launcher save picker is unavailable.");
        }
    }
    private void downloadPicked(Uri output) {
        String url = pendingDownload;
        String userAgent = pendingUserAgent;
        pendingDownload = null; pendingUserAgent = null;
        if (output == null || url == null) return;
        new Thread(() -> saveDownload(url, userAgent, output), "Hyperbolus manual download").start();
    }
    private void saveDownload(String source, String agent, Uri output) {
        HttpURLConnection connection = null;
        try {
            URL url = new URL(source);
            for (int redirects = 0; redirects <= 3; redirects++) {
                if (!"https".equalsIgnoreCase(url.getProtocol())) throw new Exception("Insecure download redirect");
                connection = (HttpURLConnection)url.openConnection();
                connection.setInstanceFollowRedirects(false);
                connection.setConnectTimeout(15000);
                connection.setReadTimeout(30000);
                if (site(Uri.parse(url.toString()))) {
                    String cookies = CookieManager.getInstance().getCookie(url.toString());
                    if (cookies != null) connection.setRequestProperty("Cookie", cookies);
                }
                if (agent != null) connection.setRequestProperty("User-Agent", agent);
                int code = connection.getResponseCode();
                if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308) {
                    String location = connection.getHeaderField("Location");
                    if (location == null) throw new Exception("Missing redirect target");
                    URL next = new URL(url, location);
                    connection.disconnect(); connection = null; url = next;
                    continue;
                }
                if (code != 200) throw new Exception("Download HTTP " + code);
                if (connection.getContentLengthLong() > 64L * 1024 * 1024) throw new Exception("Macro exceeds 64 MB");
                long total = 0;
                try (InputStream in = connection.getInputStream();
                     OutputStream out = activity.getContentResolver().openOutputStream(output, "w")) {
                    if (out == null) throw new Exception("Chosen destination is unavailable");
                    byte[] data = new byte[32768];
                    int n;
                    while ((n = in.read(data)) != -1) {
                        total += n;
                        if (total > 64L * 1024 * 1024) throw new Exception("Macro exceeds 64 MB");
                        out.write(data, 0, n);
                    }
                    out.flush();
                }
                if (total == 0) throw new Exception("Empty download");
                activity.runOnUiThread(() -> toast("Macro saved to your chosen location."));
                return;
            }
            throw new Exception("Too many download redirects");
        } catch (Exception error) {
            try { activity.getContentResolver().delete(output, null, null); } catch (Exception ignored) { }
            activity.runOnUiThread(() -> toast("Download failed: " + error.getMessage()));
        } finally {
            if (connection != null) connection.disconnect();
        }
    }
}
