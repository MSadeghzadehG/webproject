

package org.elasticsearch.test.rest.yaml;

import org.elasticsearch.test.ESIntegTestCase;

import java.util.Arrays;
import java.util.List;

import static java.util.Collections.unmodifiableList;


public final class Features {
    private static final List<String> SUPPORTED = unmodifiableList(Arrays.asList(
            "catch_unauthorized",
            "embedded_stash_key",
            "headers",
            "stash_in_key",
            "stash_in_path",
            "stash_path_replace",
            "warnings",
            "yaml"));

    private Features() {

    }

    
    public static boolean areAllSupported(List<String> features) {
        for (String feature : features) {
            if ("requires_replica".equals(feature) && ESIntegTestCase.cluster().numDataNodes() >= 2) {
                continue;
            }
            if (!SUPPORTED.contains(feature)) {
                return false;
            }
        }
        return true;
    }
}
