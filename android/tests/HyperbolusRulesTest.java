import com.jimmy1234ha.xdbot.HyperbolusRules;

public final class HyperbolusRulesTest {
    private static void check(boolean value) { if (!value) throw new AssertionError(); }
    public static void main(String[] args) {
        check(HyperbolusRules.trustedUrl("https", "hyperbolus.net", -1));
        check(HyperbolusRules.trustedUrl("HTTPS", "www.hyperbolus.net", 443));
        check(!HyperbolusRules.trustedUrl("http", "hyperbolus.net", 80));
        check(!HyperbolusRules.trustedUrl("https", "hyperbolus.net.evil.example", 443));
        check(!HyperbolusRules.trustedUrl("https", "evil.example", 443));
        check(!HyperbolusRules.trustedUrl("https", "hyperbolus.net", 8443));
        check(HyperbolusRules.trustedGdbUrl("https", "gdbrowser.com", -1));
        check(HyperbolusRules.trustedGdbUrl("HTTPS", "www.gdbrowser.com", 443));
        check(!HyperbolusRules.trustedGdbUrl("http", "gdbrowser.com", 80));
        check(!HyperbolusRules.trustedGdbUrl("https", "gdbrowser.com.evil.example", 443));
        check(!HyperbolusRules.trustedGdbUrl("https", "hyperbolus.net", 443));
        check(!HyperbolusRules.trustedGdbUrl("https", "gdbrowser.com", 8443));
        check(HyperbolusRules.supportedUpload("recording.gdr"));
        check(HyperbolusRules.supportedUpload("recording.GDR.JSON"));
        check(!HyperbolusRules.supportedUpload("recording.cml"));
        check(!HyperbolusRules.supportedUpload("recording.gdr2"));
        check(!HyperbolusRules.supportedUpload("recording.gdr2.exe"));
        check(!HyperbolusRules.supportedUpload(null));
        check(HyperbolusRules.cleanFilename("../../bad.gdr").equals(".._.._bad.gdr"));
        check(HyperbolusRules.cleanFilename("?").equals("_"));
        System.out.println("Hyperbolus rules tests passed");
    }
}
