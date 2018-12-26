

package com.google.common.collect.testing;

import static com.google.common.collect.Lists.newArrayList;
import static com.google.common.collect.testing.IteratorFeature.MODIFIABLE;
import static java.util.Collections.emptyList;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;
import junit.framework.AssertionFailedError;
import junit.framework.TestCase;


@GwtCompatible
@SuppressWarnings("serial") public class IteratorTesterTest extends TestCase {

  public void testCanCatchDifferentLengthOfIteration() {
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            4, MODIFIABLE, newArrayList(1, 2, 3), IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return Lists.newArrayList(1, 2, 3, 4).iterator();
          }
        };
    assertFailure(tester);
  }

  public void testCanCatchDifferentContents() {
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            3, MODIFIABLE, newArrayList(1, 2, 3), IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return Lists.newArrayList(1, 3, 2).iterator();
          }
        };
    assertFailure(tester);
  }

  public void testCanCatchDifferentRemoveBehaviour() {
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            3, MODIFIABLE, newArrayList(1, 2), IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return ImmutableList.of(1, 2).iterator();
          }
        };
    assertFailure(tester);
  }

  public void testUnknownOrder() {
    new IteratorTester<Integer>(
        3, MODIFIABLE, newArrayList(1, 2), IteratorTester.KnownOrder.UNKNOWN_ORDER) {
      @Override
      protected Iterator<Integer> newTargetIterator() {
        return newArrayList(2, 1).iterator();
      }
    }.test();
  }

  public void testUnknownOrderUnrecognizedElement() {
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            3, MODIFIABLE, newArrayList(1, 2, 50), IteratorTester.KnownOrder.UNKNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return newArrayList(2, 1, 3).iterator();
          }
        };
    assertFailure(tester);
  }

  
  static class IteratorWithSunJavaBug6529795<T> implements Iterator<T> {
    Iterator<T> iterator;
    boolean nextThrewException;

    IteratorWithSunJavaBug6529795(Iterator<T> iterator) {
      this.iterator = iterator;
    }

    @Override
    public boolean hasNext() {
      return iterator.hasNext();
    }

    @Override
    public T next() {
      try {
        return iterator.next();
      } catch (NoSuchElementException e) {
        nextThrewException = true;
        throw e;
      }
    }

    @Override
    public void remove() {
      if (nextThrewException) {
        throw new IllegalStateException();
      }
      iterator.remove();
    }
  }

  public void testCanCatchSunJavaBug6529795InTargetIterator() {
    try {
      
      new IteratorTester<Integer>(
          4, MODIFIABLE, newArrayList(1, 2), IteratorTester.KnownOrder.KNOWN_ORDER) {
        @Override
        protected Iterator<Integer> newTargetIterator() {
          Iterator<Integer> iterator = Lists.newArrayList(1, 2).iterator();
          return new IteratorWithSunJavaBug6529795<>(iterator);
        }
      }.test();
    } catch (AssertionFailedError e) {
      return;
    }
    fail("Should have caught jdk6 bug in target iterator");
  }

  private static final int STEPS = 3;

  static class TesterThatCountsCalls extends IteratorTester<Integer> {
    TesterThatCountsCalls() {
      super(STEPS, MODIFIABLE, newArrayList(1), IteratorTester.KnownOrder.KNOWN_ORDER);
    }

    int numCallsToNewTargetIterator;
    int numCallsToVerify;

    @Override
    protected Iterator<Integer> newTargetIterator() {
      numCallsToNewTargetIterator++;
      return Lists.newArrayList(1).iterator();
    }

    @Override
    protected void verify(List<Integer> elements) {
      numCallsToVerify++;
      super.verify(elements);
    }
  }

  public void testVerifyGetsCalled() {
    TesterThatCountsCalls tester = new TesterThatCountsCalls();

    tester.test();

    assertEquals(
        "Should have verified once per stimulus executed",
        tester.numCallsToVerify,
        tester.numCallsToNewTargetIterator * STEPS);
  }

  public void testVerifyCanThrowAssertionThatFailsTest() {
    final String message = "Important info about why verify failed";
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            1, MODIFIABLE, newArrayList(1, 2, 3), IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return Lists.newArrayList(1, 2, 3).iterator();
          }

          @Override
          protected void verify(List<Integer> elements) {
            throw new AssertionFailedError(message);
          }
        };
    AssertionFailedError actual = null;
    try {
      tester.test();
    } catch (AssertionFailedError e) {
      actual = e;
    }
    assertNotNull("verify() should be able to cause test failure", actual);
    assertTrue(
        "AssertionFailedError should have info about why test failed",
        actual.getCause().getMessage().contains(message));
  }

  public void testMissingException() {
    List<Integer> emptyList = newArrayList();

    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            1, MODIFIABLE, emptyList, IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return new Iterator<Integer>() {
              @Override
              public void remove() {
                              }

              @Override
              public Integer next() {
                                return null;
              }

              @Override
              public boolean hasNext() {
                return false;
              }
            };
          }
        };
    assertFailure(tester);
  }

  public void testUnexpectedException() {
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            1, MODIFIABLE, newArrayList(1), IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return new ThrowingIterator<>(new IllegalStateException());
          }
        };
    assertFailure(tester);
  }

  public void testSimilarException() {
    List<Integer> emptyList = emptyList();
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            1, MODIFIABLE, emptyList, IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return new Iterator<Integer>() {
              @Override
              public void remove() {
                throw new IllegalStateException() {
                  
                };
              }

              @Override
              public Integer next() {
                throw new NoSuchElementException() {
                  
                };
              }

              @Override
              public boolean hasNext() {
                return false;
              }
            };
          }
        };
    tester.test();
  }

  public void testMismatchedException() {
    List<Integer> emptyList = emptyList();
    IteratorTester<Integer> tester =
        new IteratorTester<Integer>(
            1, MODIFIABLE, emptyList, IteratorTester.KnownOrder.KNOWN_ORDER) {
          @Override
          protected Iterator<Integer> newTargetIterator() {
            return new Iterator<Integer>() {
              @Override
              public void remove() {
                                throw new IllegalArgumentException();
              }

              @Override
              public Integer next() {
                                throw new UnsupportedOperationException();
              }

              @Override
              public boolean hasNext() {
                return false;
              }
            };
          }
        };
    assertFailure(tester);
  }

  private static void assertFailure(IteratorTester<?> tester) {
    try {
      tester.test();
    } catch (AssertionFailedError expected) {
      return;
    }
    fail();
  }

  private static final class ThrowingIterator<E> implements Iterator<E> {
    private final RuntimeException ex;

    private ThrowingIterator(RuntimeException ex) {
      this.ex = ex;
    }

    @Override
    public boolean hasNext() {
            return true;
    }

    @Override
    public E next() {
      ex.fillInStackTrace();
      throw ex;
    }

    @Override
    public void remove() {
      ex.fillInStackTrace();
      throw ex;
    }
  }
}
