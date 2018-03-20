

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionSize.ZERO;
import static com.google.common.collect.testing.features.ListFeature.SUPPORTS_SET;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.ListFeature;
import java.util.Collections;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class ListReplaceAllTester<E> extends AbstractListTester<E> {
  @ListFeature.Require(SUPPORTS_SET)
  public void testReplaceAll() {
    getList().replaceAll(e -> samples.e3());
    expectContents(Collections.nCopies(getNumElements(), samples.e3()));
  }

  @ListFeature.Require(SUPPORTS_SET)
  public void testReplaceAll_changesSome() {
    getList().replaceAll(e -> (e.equals(samples.e0())) ? samples.e3() : e);
    E[] expected = createSamplesArray();
    for (int i = 0; i < expected.length; i++) {
      if (expected[i].equals(samples.e0())) {
        expected[i] = samples.e3();
      }
    }
    expectContents(expected);
  }

  @CollectionSize.Require(absent = ZERO)
  @ListFeature.Require(absent = SUPPORTS_SET)
  public void testReplaceAll_unsupported() {
    try {
      getList().replaceAll(e -> e);
      fail("replaceAll() should throw UnsupportedOperationException");
    } catch (UnsupportedOperationException expected) {
    }
    expectUnchanged();
  }
}
