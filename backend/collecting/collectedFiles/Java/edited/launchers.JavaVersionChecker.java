

package org.elasticsearch.tools.launchers;

import java.util.Arrays;
import java.util.Locale;


final class JavaVersionChecker {

    private JavaVersionChecker() {
    }

    
    public static void main(final String[] args) {
                if (args.length != 0) {
            throw new IllegalArgumentException("expected zero arguments but was " + Arrays.toString(args));
        }
        if (JavaVersion.compare(JavaVersion.CURRENT, JavaVersion.JAVA_8) < 0) {
            final String message = String.format(
                    Locale.ROOT,
                    "the minimum required Java version is 8; your Java version from [%s] does not meet this requirement",
                    System.getProperty("java.home"));
            Launchers.errPrintln(message);
            Launchers.exit(1);
        }
        Launchers.exit(0);
    }

}
