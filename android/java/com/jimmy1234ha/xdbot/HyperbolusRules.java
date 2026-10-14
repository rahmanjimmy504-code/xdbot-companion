package com.jimmy1234ha.xdbot;

import java.util.Locale;

/** Security policy shared by the native browser and host tests. */
public final class HyperbolusRules {
    private HyperbolusRules() { }

    public static boolean trustedUrl(String scheme, String host, int port) {
        return scheme != null && scheme.equalsIgnoreCase("https") && host != null &&
            (host.equalsIgnoreCase("hyperbolus.net") || host.equalsIgnoreCase("www.hyperbolus.net")) &&
            (port == -1 || port == 443);
    }
    public static boolean supportedUpload(String filename) {
        if (filename == null) return false;
        String lower = filename.toLowerCase(Locale.ROOT);
        return lower.endsWith(".gdr") || lower.endsWith(".json");
    }
    public static String cleanFilename(String name) {
        if (name == null) return "hyperbolus-macro.gdr";
        String safe = name.replaceAll("[^a-zA-Z0-9._ -]", "_");
        return safe.isEmpty() || safe.equals(".") || safe.equals("..") ? "hyperbolus-macro.gdr" : safe;
    }
}
