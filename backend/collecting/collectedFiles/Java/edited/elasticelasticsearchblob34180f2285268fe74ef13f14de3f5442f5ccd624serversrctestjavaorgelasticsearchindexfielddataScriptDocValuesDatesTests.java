

package org.elasticsearch.index.fielddata;

import org.elasticsearch.index.fielddata.ScriptDocValues.Dates;
import org.elasticsearch.test.ESTestCase;
import org.joda.time.DateTime;
import org.joda.time.DateTimeZone;
import org.joda.time.ReadableDateTime;

import java.io.IOException;
import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.PrivilegedAction;
import java.security.ProtectionDomain;
import java.util.HashSet;
import java.util.Set;
import java.util.function.Consumer;

import static org.hamcrest.Matchers.containsInAnyOrder;

public class ScriptDocValuesDatesTests extends ESTestCase {
    public void test() throws IOException {
        long[][] values = new long[between(3, 10)][];
        ReadableDateTime[][] expectedDates = new ReadableDateTime[values.length][];
        for (int d = 0; d < values.length; d++) {
            values[d] = new long[randomBoolean() ? randomBoolean() ? 0 : 1 : between(2, 100)];
            expectedDates[d] = new ReadableDateTime[values[d].length];
            for (int i = 0; i < values[d].length; i++) {
                expectedDates[d][i] = new DateTime(randomNonNegativeLong(), DateTimeZone.UTC);
                values[d][i] = expectedDates[d][i].getMillis();
            }
        }
        Dates dates = wrap(values);
        for (int round = 0; round < 10; round++) {
            int d = between(0, values.length - 1);
            dates.setNextDocId(d);
            assertEquals(expectedDates[d].length > 0 ? expectedDates[d][0] : new DateTime(0, DateTimeZone.UTC), dates.getValue());

            assertEquals(values[d].length, dates.size());
            for (int i = 0; i < values[d].length; i++) {
                assertEquals(expectedDates[d][i], dates.get(i));
            }

            Exception e = expectThrows(UnsupportedOperationException.class, () -> dates.add(new DateTime()));
            assertEquals("doc values are unmodifiable", e.getMessage());
        }
    }

    private Dates wrap(long[][] values) {
        return new Dates(new AbstractSortedNumericDocValues() {
            long[] current;
            int i;

            @Override
            public boolean advanceExact(int doc) {
                current = values[doc];
                i = 0;
                return current.length > 0;
            }
            @Override
            public int docValueCount() {
                return current.length;
            }
            @Override
            public long nextValue() {
                return current[i++];
            }
        });
    }
}
