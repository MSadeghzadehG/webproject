

package org.elasticsearch.ingest;

import java.util.List;
import java.util.Locale;
import java.util.Map;

import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertThat;

public class IngestDocumentMatcher {
    
    public static void assertIngestDocument(Object a, Object b) {
        if (a instanceof Map) {
            Map<?, ?> mapA = (Map<?, ?>) a;
            Map<?, ?> mapB = (Map<?, ?>) b;
            for (Map.Entry<?, ?> entry : mapA.entrySet()) {
                if (entry.getValue() instanceof List || entry.getValue() instanceof Map) {
                    assertIngestDocument(entry.getValue(), mapB.get(entry.getKey()));
                }
            }
        } else if (a instanceof List) {
            List<?> listA = (List<?>) a;
            List<?> listB = (List<?>) b;
            for (int i = 0; i < listA.size(); i++) {
                Object value = listA.get(i);
                if (value instanceof List || value instanceof Map) {
                    assertIngestDocument(value, listB.get(i));
                }
            }
        } else if (a instanceof byte[]) {
            assertArrayEquals((byte[]) a, (byte[])b);
        } else if (a instanceof IngestDocument) {
            IngestDocument docA = (IngestDocument) a;
            IngestDocument docB = (IngestDocument) b;
            assertIngestDocument(docA.getSourceAndMetadata(), docB.getSourceAndMetadata());
            assertIngestDocument(docA.getIngestMetadata(), docB.getIngestMetadata());
        } else {
            String msg = String.format(Locale.ROOT, "Expected %s class to be equal to %s", a.getClass().getName(), b.getClass().getName());
            assertThat(msg, a, equalTo(b));
        }
    }
}
