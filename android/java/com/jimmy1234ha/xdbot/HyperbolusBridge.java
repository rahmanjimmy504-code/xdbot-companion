package com.jimmy1234ha.xdbot;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
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
import java.io.InputStream;
import java.io.OutputStream;
import android.app.Fragment;
import android.app.FragmentManager;
import java.net.HttpURLConnection;
import java.net.URL;

/** Native Android WebView over Geometry Dash. No password or session is passed to native mod code. */
public final class HyperbolusBridge extends WebChromeClient implements DownloadListener {
    private static HyperbolusBridge active;
    private final Activity activity;
    private final Uri selectedMacro;
    private final String selectedName;
    private final boolean gdbMode;
    private Boolean switchToGdb;
    private WebView web;
    private AlertDialog dialog;
    private static Uri pendingExportSource;
    private static final int PICK_UPLOAD = 501;
    private static final int PICK_DOWNLOAD = 502;
    private static final int PICK_EXPORT = 503;
    private static final String DOCUMENT_TAG = "xdbot-companion-documents";
    private ValueCallback<Uri[]> fileCallback;
    private String pendingDownload;
    private String pendingUserAgent;
    private String websiteStage = "before WebView setup";

    private HyperbolusBridge(Activity activity, Uri selectedMacro, String selectedName, boolean gdbMode) {
        this.activity = activity;
        this.selectedMacro = selectedMacro;
        this.selectedName = selectedName;
        this.gdbMode = gdbMode;
    }

    public static void open(Activity activity, Uri macro, String name) {
        openMode(activity, macro, name, false);
    }

    // Distinct browser instance: no local macro is offered on GDBrowser.
    public static void openGDB(Activity activity, Uri unused, String unusedName) {
        openMode(activity, null, "", true);
    }

    private static void openMode(Activity activity, Uri macro, String name, boolean gdbMode) {
        activity.runOnUiThread(() -> {
            if (active != null) {
                if (active.dialog != null && active.gdbMode != gdbMode) active.switchSite(gdbMode);
                else if (active.dialog != null && !active.dialog.isShowing()) active.dialog.show();
                return;
            }
            HyperbolusBridge bridge = new HyperbolusBridge(activity, macro, name, gdbMode);
            // Do not depend on any picker setup just to open the website.
            active = bridge;
            try { bridge.show(); }
            catch (RuntimeException e) {
                // Beta.6 discarded the exception message, hiding whether the
                // WebView provider or the Android window actually failed.
                android.util.Log.e("XdBotHyperbolus", "Website stage: " + bridge.websiteStage, e);
                active = null;
                try {
                    if (bridge.dialog != null && bridge.dialog.isShowing()) bridge.dialog.dismiss();
                    else if (bridge.web != null) bridge.web.destroy();
                } catch (RuntimeException ignored) { }
                String detail = e.getMessage();
                if (detail == null && e.getCause() != null) detail = e.getCause().toString();
                if (detail == null) detail = "No additional message; check Android logs.";
                if (detail.length() > 600) detail = detail.substring(0, 600);
                String message = "Stage: " + bridge.websiteStage + "\n" + e.getClass().getName() + "\n" + detail;
                try {
                    new AlertDialog.Builder(activity).setTitle("HYPE diagnostic")
                        .setMessage(message).setPositiveButton("OK", (d, which) -> { }).show();
                } catch (RuntimeException dialogError) {
                    Toast.makeText(activity, message, Toast.LENGTH_LONG).show();
                }
            }
        });
    }

    /** Save a local xdBot macro to the URI the user chose, without a WebView. */
    public static void export(Activity activity, Uri macro, String filename) {
        activity.runOnUiThread(() -> {
            if (macro == null) {
                Toast.makeText(activity, "Could not read the selected macro.", Toast.LENGTH_LONG).show();
                return;
            }
            if (pendingExportSource != null) {
                Toast.makeText(activity, "Finish the current export first.", Toast.LENGTH_LONG).show();
                return;
            }
            try {
                DocumentFragment receiver = documents(activity);
                Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT)
                    .addCategory(Intent.CATEGORY_OPENABLE)
                    .setType("application/octet-stream")
                    .putExtra(Intent.EXTRA_TITLE, HyperbolusRules.cleanFilename(filename))
                    .addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                pendingExportSource = macro;
                receiver.startActivityForResult(intent, PICK_EXPORT);
            } catch (RuntimeException e) {
                pendingExportSource = null;
                Toast.makeText(activity, "Export picker failed: " + e.getClass().getSimpleName(), Toast.LENGTH_LONG).show();
            }
        });
    }
    private static void copyLocal(Activity activity, Uri source, Uri output) {
        try (InputStream in = activity.getContentResolver().openInputStream(source);
             OutputStream out = activity.getContentResolver().openOutputStream(output, "wt")) {
            if (in == null || out == null) throw new Exception("Selected file cannot be opened");
            long total = 0;
            byte[] buffer = new byte[32768];
            int n;
            while ((n = in.read(buffer)) != -1) {
                total += n;
                if (total > 64L * 1024 * 1024) throw new Exception("Macro exceeds 64 MB");
                out.write(buffer, 0, n);
            }
            out.flush();
            if (total == 0) throw new Exception("The macro was empty");
            activity.runOnUiThread(() -> Toast.makeText(activity, "Macro exported to chosen location.", Toast.LENGTH_LONG).show());
        } catch (Exception e) {
            try { activity.getContentResolver().delete(output, null, null); } catch (Exception ignored) { }
            activity.runOnUiThread(() -> Toast.makeText(activity, "Export failed: " + e.getMessage(), Toast.LENGTH_LONG).show());
        }
    }

    private static boolean site(Uri uri) {
        return uri != null && HyperbolusRules.trustedUrl(uri.getScheme(), uri.getHost(), uri.getPort());
    }
    private static boolean gdbSite(Uri uri) {
        return uri != null && HyperbolusRules.trustedGdbUrl(uri.getScheme(), uri.getHost(), uri.getPort());
    }
    private boolean allowed(Uri uri) { return gdbMode ? gdbSite(uri) : site(uri); }
    private void toast(String message) { Toast.makeText(activity, message, Toast.LENGTH_LONG).show(); }

    // Framework Fragment is used deliberately: the released Geode launcher
    // minifies AndroidX result classes and private Geode picker contracts. This
    // Android SDK API cannot be renamed by the launcher's R8 configuration.
    public static final class DocumentFragment extends Fragment {
        public DocumentFragment() { }
        @Override public void onActivityResult(int request, int resultCode, Intent data) {
            super.onActivityResult(request, resultCode, data);
            Uri uri = resultCode == Activity.RESULT_OK && data != null ? data.getData() : null;
            if (request == PICK_UPLOAD && active != null) active.uploadPicked(uri);
            else if (request == PICK_DOWNLOAD && active != null) active.downloadPicked(uri);
            else if (request == PICK_EXPORT) {
                Uri source = pendingExportSource;
                pendingExportSource = null;
                Activity current = getActivity();
                if (source != null && uri != null && current != null)
                    new Thread(() -> copyLocal(current, source, uri), "xdBot macro export").start();
            }
            // Do not leave an unnecessary dynamic-dex fragment in the host
            // activity's saved state after a completed picker operation.
            if (getFragmentManager() != null)
                getFragmentManager().beginTransaction().remove(this).commitAllowingStateLoss();
        }
    }
    private static DocumentFragment documents(Activity activity) {
        FragmentManager manager = activity.getFragmentManager();
        Fragment existing = manager.findFragmentByTag(DOCUMENT_TAG);
        if (existing instanceof DocumentFragment) return (DocumentFragment)existing;
        if (existing != null) throw new IllegalStateException("Picker tag is already used");
        DocumentFragment receiver = new DocumentFragment();
        receiver.setRetainInstance(true); // Keep outstanding results across configuration changes.
        manager.beginTransaction().add(receiver, DOCUMENT_TAG).commit();
        manager.executePendingTransactions();
        return receiver;
    }
    private void launchOpen() {
        try {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .setType("*/*")
                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            documents(activity).startActivityForResult(intent, PICK_UPLOAD);
        } catch (RuntimeException e) {
            toast("Could not open the Android file picker.");
            uploadPicked(null);
        }
    }
    private void launchSave(String name) {
        try {
            Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .setType("application/octet-stream")
                .putExtra(Intent.EXTRA_TITLE, name)
                .addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            documents(activity).startActivityForResult(intent, PICK_DOWNLOAD);
        } catch (RuntimeException e) {
            pendingDownload = null; pendingUserAgent = null;
            toast("Could not open the Android save picker.");
        }
    }

    // AlertDialog checks its custom view tree for a text editor before it
    // connects its window to the IME. A plain WebView may report false until
    // an HTML input already has focus, leaving the dialog unable to type.
    public static final class InputFriendlyWebView extends WebView {
        public InputFriendlyWebView(Activity activity) { super(activity); }
        @Override public boolean onCheckIsTextEditor() { return true; }
    }

    private void show() {
        websiteStage = "construct WebView";
        web = new InputFriendlyWebView(activity);
        web.setFocusable(true);
        web.setFocusableInTouchMode(true);
        websiteStage = "configure WebView";
        WebSettings settings = web.getSettings();
        settings.setJavaScriptEnabled(true); // Hyperbolus's own site needs JavaScript.
        settings.setDomStorageEnabled(true);
        if (gdbMode) {
            // GDBrowser is a landscape desktop-style site. Fit its wide layout
            // inside the enlarged dialog without changing Hyperbolus's layout.
            settings.setUseWideViewPort(true);
            settings.setLoadWithOverviewMode(true);
        }
        settings.setAllowFileAccess(false);
        settings.setAllowContentAccess(true); // Needed for user-selected SAF uploads.
        settings.setJavaScriptCanOpenWindowsAutomatically(false);
        settings.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);
        websiteStage = "configure WebView cookies";
        CookieManager.getInstance().setAcceptThirdPartyCookies(web, false);
        websiteStage = "configure WebView security";
        settings.setAllowFileAccessFromFileURLs(false);
        settings.setAllowUniversalAccessFromFileURLs(false);
        if (Build.VERSION.SDK_INT >= 26) settings.setSafeBrowsingEnabled(true);
        websiteStage = "connect WebView callbacks";
        web.setWebChromeClient(this);
        web.setDownloadListener(this);
        web.setWebViewClient(new WebViewClient() {
            @Override public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                if (request.isForMainFrame()) {
                    if (!allowed(request.getUrl())) {
                        toast("Blocked a link outside this browser's website."); return true;
                    }
                    // Hyperbolus download links redirect to signed storage;
                    // capture them before the WebView leaves the trusted site.
                    if (!gdbMode && request.getUrl().getPath() != null &&
                        request.getUrl().getPath().startsWith("/download/")) {
                        queueDownload(request.getUrl().toString(), view.getSettings().getUserAgentString(),
                            "hyperbolus-macro");
                        return true;
                    }
                }
                return false;
            }
            @Override public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                handler.cancel(); toast("Website TLS certificate check failed.");
            }
        });
        websiteStage = "create browser controls";
        LinearLayout content = new LinearLayout(activity);
        content.setOrientation(LinearLayout.VERTICAL);
        LinearLayout nav = new LinearLayout(activity);
        nav.setOrientation(LinearLayout.HORIZONTAL);
        if (gdbMode) {
            addNav(nav, "GDB HOME", "https://gdbrowser.com/", android.R.drawable.ic_menu_search);
            addSiteSwitch(nav, "HYPE", false, android.R.drawable.ic_menu_view);
        } else {
            addNav(nav, "LOGIN", "https://hyperbolus.net/auth/login", android.R.drawable.ic_lock_lock);
            addNav(nav, "REPLAYS", "https://hyperbolus.net/replays", android.R.drawable.ic_menu_agenda);
            addNav(nav, "UPLOAD", "https://hyperbolus.net/replays/new", android.R.drawable.ic_menu_upload);
            addSiteSwitch(nav, "GDB", true, android.R.drawable.ic_menu_search);
        }
        Button back = new Button(activity);
        back.setText("BACK");
        back.setCompoundDrawablesWithIntrinsicBounds(android.R.drawable.ic_media_previous, 0, 0, 0);
        back.setCompoundDrawablePadding(4);
        back.setOnClickListener(v -> { if (web.canGoBack()) web.goBack(); });
        nav.addView(back, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        content.addView(nav);
        int height = (int)(activity.getResources().getDisplayMetrics().heightPixels * (gdbMode ? .58f : .70f));
        content.addView(web, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, height));
        websiteStage = "create website dialog";
        dialog = new AlertDialog.Builder(activity)
            .setTitle(gdbMode ? "GD Browser: manual website" : "Hyperbolus: manual website")
            .setView(content).setNegativeButton("CLOSE", (d, which) -> { }).create();
        dialog.setOnDismissListener(d -> close());
        websiteStage = "show website dialog";
        dialog.show();
        websiteStage = "configure website keyboard";
        if (dialog.getWindow() != null) {
            if (gdbMode) {
                // Screenshot on Fire tablet: a default AlertDialog was only
                // ~670px wide on a 1568px landscape display. GDBrowser sees
                // the *WebView* as portrait and blocks the page. Give it most
                // of the available width, while keeping Android's own close,
                // navigation and file-picking controls visible.
                int width = (int)(activity.getResources().getDisplayMetrics().widthPixels * .94f);
                dialog.getWindow().setLayout(width, WindowManager.LayoutParams.WRAP_CONTENT);
            }
            // The full-screen launcher hides its game keyboard. Keep this
            // dialog's own window focusable and attached to Android's IME.
            dialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
            dialog.getWindow().setSoftInputMode(
                WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE |
                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        }
        websiteStage = "load website home page";
        web.loadUrl(gdbMode ? "https://gdbrowser.com/" : "https://hyperbolus.net/auth/login");
        websiteStage = "website opened";
    }
    private void addNav(LinearLayout row, String label, String url, int icon) {
        Button button = new Button(activity);
        button.setText(label);
        button.setCompoundDrawablesWithIntrinsicBounds(icon, 0, 0, 0);
        button.setCompoundDrawablePadding(4);
        button.setOnClickListener(v -> web.loadUrl(url));
        row.addView(button, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
    }
    private void addSiteSwitch(LinearLayout row, String label, boolean toGdb, int icon) {
        Button button = new Button(activity);
        button.setText(label);
        button.setCompoundDrawablesWithIntrinsicBounds(icon, 0, 0, 0);
        button.setCompoundDrawablePadding(4);
        button.setOnClickListener(v -> switchSite(toGdb));
        row.addView(button, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
    }
    private void switchSite(boolean toGdb) {
        if (gdbMode == toGdb || dialog == null) return;
        switchToGdb = toGdb;
        dialog.dismiss(); // The dismiss callback creates a fresh, separate WebView.
    }
    private void close() {
        Boolean next = switchToGdb;
        switchToGdb = null;
        if (fileCallback != null) { fileCallback.onReceiveValue(null); fileCallback = null; }
        if (web != null) { web.stopLoading(); web.destroy(); web = null; }
        dialog = null;
        active = null;
        // Browser cookies remain scoped to each website. No mod-owned password field.
        if (next != null) openMode(activity, null, "", next);
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
    private void deliverUpload(Uri uri) {
        if (fileCallback == null) return;
        ValueCallback<Uri[]> callback = fileCallback;
        fileCallback = null;
        callback.onReceiveValue(uri == null ? null : new Uri[]{uri});
    }
    private void uploadPicked(Uri uri) {
        if (fileCallback == null) return;
        if (uri != null && !gdbMode) {
            // The companion already validated a preselected macro's filename;
            // its content provider may not expose DISPLAY_NAME on every ROM.
            // Manually chosen documents must provide a readable display name.
            boolean preselected = selectedMacro != null && uri.equals(selectedMacro);
            String name = preselected ? selectedName : documentName(uri);
            if (!supportedUpload(name)) {
                toast("Unsupported for Hyperbolus: choose a .gdr or .json file. .cml and .gdr2 are not accepted.");
                deliverUpload(null);
                return;
            }
            // Preselected macros have already passed the ATTACH confirmation
            // below. A manually picked document must get its own confirmation
            // before the URI is returned to the website's file input.
            if (!preselected) {
                new AlertDialog.Builder(activity).setTitle("Attach chosen replay?")
                    .setMessage(name + "\nOnly this file will be offered to Hyperbolus. Check the filename before continuing; the website may process it after attachment.")
                    .setPositiveButton("ATTACH", (d, which) -> deliverUpload(uri))
                    .setNegativeButton("CANCEL", (d, which) -> deliverUpload(null))
                    .setOnCancelListener(d -> deliverUpload(null)).show();
                return;
            }
        }
        deliverUpload(uri);
    }
    @Override public boolean onShowFileChooser(WebView view, ValueCallback<Uri[]> callback, FileChooserParams params) {
        Uri page = Uri.parse(view.getUrl() == null ? "" : view.getUrl());
        if (gdbMode) {
            if (!gdbSite(page)) {
                toast("Choose files only while on gdbrowser.com.");
                callback.onReceiveValue(null); return true;
            }
            if (fileCallback != null) fileCallback.onReceiveValue(null);
            fileCallback = callback;
            new AlertDialog.Builder(activity).setTitle("Choose a file for GD Browser?")
                .setMessage("The companion will not pick a file for you. This website may process a file after you choose it. Continue to Android's file picker?")
                .setPositiveButton("CHOOSE FILE", (d, which) -> launchOpen())
                .setNegativeButton("CANCEL", (d, which) -> uploadPicked(null))
                .setOnCancelListener(d -> uploadPicked(null)).show();
            return true;
        }
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
            launchOpen();
        }
        return true;
    }

    @Override public void onDownloadStart(String url, String userAgent, String disposition, String mime, long length) {
        // Downloads must originate on the active site. No site cookies are sent
        // to signed or unrelated external HTTPS storage during redirect follow.
        if (!allowed(Uri.parse(url))) { toast("Blocked a download outside this browser's website."); return; }
        String name = HyperbolusRules.cleanFilename(URLUtil.guessFileName(url, disposition, mime));
        queueDownload(url, userAgent, name);
    }
    private void queueDownload(String url, String userAgent, String name) {
        if (pendingDownload != null) {
            toast("Finish the current download picker first."); return;
        }
        pendingDownload = url;
        pendingUserAgent = userAgent;
        launchSave(name);
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
                if (allowed(Uri.parse(url.toString()))) {
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
                     OutputStream out = activity.getContentResolver().openOutputStream(output, "wt")) {
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
                activity.runOnUiThread(() -> toast("File saved to your chosen location."));
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
