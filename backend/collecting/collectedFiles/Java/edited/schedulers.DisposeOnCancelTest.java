

package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.disposables.*;

public class DisposeOnCancelTest {

    @Test
    public void basicCoverage() throws Exception {
        Disposable d = Disposables.empty();

        DisposeOnCancel doc = new DisposeOnCancel(d);

        assertFalse(doc.cancel(true));

        assertFalse(doc.isCancelled());

        assertFalse(doc.isDone());

        assertNull(doc.get());

        assertNull(doc.get(1, TimeUnit.SECONDS));
    }
}
