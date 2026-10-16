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
            // Do not depend on any picker setup just to open the website.
            active = bridge;
            try { bridge.show(); }
            catch (RuntimeException e) {
                active = null;
                Toast.makeText(activity, "Website window failed: " + e.getClass().getSimpleName(), Toast.LENGTH_LONG).show();
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
            launchOpen();
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
