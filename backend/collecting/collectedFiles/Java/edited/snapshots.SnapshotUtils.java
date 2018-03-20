
package org.elasticsearch.snapshots;

import org.elasticsearch.action.support.IndicesOptions;
import org.elasticsearch.cluster.metadata.IndexNameExpressionResolver;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.index.IndexNotFoundException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


public class SnapshotUtils {

    
    public static List<String> filterIndices(List<String> availableIndices, String[] selectedIndices, IndicesOptions indicesOptions) {
        if (IndexNameExpressionResolver.isAllIndices(Arrays.asList(selectedIndices))) {
            return availableIndices;
        }
        Set<String> result = null;
        for (int i = 0; i < selectedIndices.length; i++) {
            String indexOrPattern = selectedIndices[i];
            boolean add = true;
            if (!indexOrPattern.isEmpty()) {
                if (availableIndices.contains(indexOrPattern)) {
                    if (result == null) {
                        result = new HashSet<>();
                    }
                    result.add(indexOrPattern);
                    continue;
                }
                if (indexOrPattern.charAt(0) == '+') {
                    add = true;
                    indexOrPattern = indexOrPattern.substring(1);
                                        if (i == 0) {
                        result = new HashSet<>();
                    }
                } else if (indexOrPattern.charAt(0) == '-') {
                                        if (i == 0) {
                        result = new HashSet<>(availableIndices);
                    }
                    add = false;
                    indexOrPattern = indexOrPattern.substring(1);
                }
            }
            if (indexOrPattern.isEmpty() || !Regex.isSimpleMatchPattern(indexOrPattern)) {
                if (!availableIndices.contains(indexOrPattern)) {
                    if (!indicesOptions.ignoreUnavailable()) {
                        throw new IndexNotFoundException(indexOrPattern);
                    } else {
                        if (result == null) {
                                                        result = new HashSet<>(availableIndices.subList(0, i));
                        }
                    }
                } else {
                    if (result != null) {
                        if (add) {
                            result.add(indexOrPattern);
                        } else {
                            result.remove(indexOrPattern);
                        }
                    }
                }
                continue;
            }
            if (result == null) {
                                result = new HashSet<>(availableIndices.subList(0, i));
            }
            boolean found = false;
            for (String index : availableIndices) {
                if (Regex.simpleMatch(indexOrPattern, index)) {
                    found = true;
                    if (add) {
                        result.add(index);
                    } else {
                        result.remove(index);
                    }
                }
            }
            if (!found && !indicesOptions.allowNoIndices()) {
                throw new IndexNotFoundException(indexOrPattern);
            }
        }
        if (result == null) {
            return Collections.unmodifiableList(new ArrayList<>(Arrays.asList(selectedIndices)));
        }
        return Collections.unmodifiableList(new ArrayList<>(result));
    }
}
