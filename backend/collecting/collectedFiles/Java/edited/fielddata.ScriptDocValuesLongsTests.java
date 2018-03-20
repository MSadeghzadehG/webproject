

package org.elasticsearch.index.fielddata;

import org.elasticsearch.index.fielddata.ScriptDocValues.Longs;
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

public class ScriptDocValuesLongsTests extends ESTestCase {
    public void testLongs() throws IOException {
        long[][] values = new long[between(3, 10)][];
        for (int d = 0; d < values.length; d++) {
            values[d] = new long[randomBoolean() ? randomBoolean() ? 0 : 1 : between(2, 100)];
            for (int i = 0; i < values[d].length; i++) {
                values[d][i] = randomLong();
            }
        }
        Longs longs = wrap(values, deprecationMessage -> {fail("unexpected deprecation: " + deprecationMessage);});

        for (int round = 0; round < 10; round++) {
            int d = between(0, values.length - 1);
            longs.setNextDocId(d);
            assertEquals(values[d].length > 0 ? values[d][0] : 0, longs.getValue());

            assertEquals(values[d].length, longs.size());
            assertEquals(values[d].length, longs.getValues().size());
            for (int i = 0; i < values[d].length; i++) {
                assertEquals(values[d][i], longs.get(i).longValue());
                assertEquals(values[d][i], longs.getValues().get(i).longValue());
            }

            Exception e = expectThrows(UnsupportedOperationException.class, () -> longs.getValues().add(100L));
            assertEquals("doc values are unmodifiable", e.getMessage());
        }
    }

    public void testDates() throws IOException {
        long[][] values = new long[between(3, 10)][];
        ReadableDateTime[][] dates = new ReadableDateTime[values.length][];
        for (int d = 0; d < values.length; d++) {
            values[d] = new long[randomBoolean() ? randomBoolean() ? 0 : 1 : between(2, 100)];
            dates[d] = new ReadableDateTime[values[d].length];
            for (int i = 0; i < values[d].length; i++) {
                dates[d][i] = new DateTime(randomNonNegativeLong(), DateTimeZone.UTC);
                values[d][i] = dates[d][i].getMillis();
            }
        }
        Set<String> warnings = new HashSet<>();
        Longs longs = wrap(values, deprecationMessage -> {
            warnings.add(deprecationMessage);
            
            createTempDir();
        });

        for (int round = 0; round < 10; round++) {
            int d = between(0, values.length - 1);
            longs.setNextDocId(d);
            assertEquals(dates[d].length > 0 ? dates[d][0] : new DateTime(0, DateTimeZone.UTC), longs.getDate());

            assertEquals(values[d].length, longs.getDates().size());
            for (int i = 0; i < values[d].length; i++) {
                assertEquals(dates[d][i], longs.getDates().get(i));
            }

            Exception e = expectThrows(UnsupportedOperationException.class, () -> longs.getDates().add(new DateTime()));
            assertEquals("doc values are unmodifiable", e.getMessage());
        }

        
        PermissionCollection noPermissions = new Permissions();
        AccessControlContext noPermissionsAcc = new AccessControlContext(
            new ProtectionDomain[] {
                new ProtectionDomain(null, noPermissions)
            }
        );
        AccessController.doPrivileged(new PrivilegedAction<Void>() {
            public Void run() {
                try {
                    longs.getDates();
                } catch (IOException e) {
                    throw new RuntimeException("unexpected", e);
                }
                return null;
            }
        }, noPermissionsAcc);

        assertThat(warnings, containsInAnyOrder(
                "getDate on numeric fields is deprecated. Use a date field to get dates.",
                "getDates on numeric fields is deprecated. Use a date field to get dates."));
    }

    private Longs wrap(long[][] values, Consumer<String> deprecationCallback) {
        return new Longs(new AbstractSortedNumericDocValues() {
            long[] current;
            int i;

            @Override
            public boolean advanceExact(int doc) {
                i = 0;
                current = values[doc];
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
        }, deprecationCallback);
    }
}
