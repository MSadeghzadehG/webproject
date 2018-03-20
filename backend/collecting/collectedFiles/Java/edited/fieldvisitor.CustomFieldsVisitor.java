
package org.elasticsearch.index.fieldvisitor;

import org.apache.lucene.index.FieldInfo;
import org.elasticsearch.common.regex.Regex;

import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Set;


public class CustomFieldsVisitor extends FieldsVisitor {

    private final Set<String> fields;
    private final List<String> patterns;

    public CustomFieldsVisitor(Set<String> fields, List<String> patterns, boolean loadSource) {
        super(loadSource);
        this.fields = fields;
        this.patterns = patterns;
    }

    public CustomFieldsVisitor(Set<String> fields, boolean loadSource) {
        this(fields, Collections.emptyList(), loadSource);
    }

    @Override
    public Status needsField(FieldInfo fieldInfo) throws IOException {
        if (super.needsField(fieldInfo) == Status.YES) {
            return Status.YES;
        }
        if (fields.contains(fieldInfo.name)) {
            return Status.YES;
        }
        for (String pattern : patterns) {
            if (Regex.simpleMatch(pattern, fieldInfo.name)) {
                return Status.YES;
            }
        }
        return Status.NO;
    }
}
