

package com.google.common.util.concurrent;

import static com.google.common.collect.Iterables.concat;

import com.google.common.base.Functions;
import com.google.common.base.Supplier;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Ordering;
import com.google.common.collect.Sets;
import com.google.common.testing.GcFinalization;
import com.google.common.testing.NullPointerTester;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import junit.framework.TestCase;


public class StripedTest extends TestCase {
  private static List<Striped<?>> strongImplementations() {
    return ImmutableList.of(
        Striped.readWriteLock(100),
        Striped.readWriteLock(256),
        Striped.lock(100),
        Striped.lock(256),
        Striped.semaphore(100, 1),
        Striped.semaphore(256, 1));
  }

  private static final Supplier<ReadWriteLock> READ_WRITE_LOCK_SUPPLIER =
      new Supplier<ReadWriteLock>() {
        @Override
        public ReadWriteLock get() {
          return new ReentrantReadWriteLock();
        }
      };

  private static final Supplier<Lock> LOCK_SUPPLER =
      new Supplier<Lock>() {
        @Override
        public Lock get() {
          return new ReentrantLock();
        }
      };

  private static final Supplier<Semaphore> SEMAPHORE_SUPPLER =
      new Supplier<Semaphore>() {
        @Override
        public Semaphore get() {
          return new Semaphore(1, false);
        }
      };

  private static List<Striped<?>> weakImplementations() {
    return ImmutableList.<Striped<?>>builder()
        .add(new Striped.SmallLazyStriped<ReadWriteLock>(50, READ_WRITE_LOCK_SUPPLIER))
        .add(new Striped.SmallLazyStriped<ReadWriteLock>(64, READ_WRITE_LOCK_SUPPLIER))
        .add(new Striped.LargeLazyStriped<ReadWriteLock>(50, READ_WRITE_LOCK_SUPPLIER))
        .add(new Striped.LargeLazyStriped<ReadWriteLock>(64, READ_WRITE_LOCK_SUPPLIER))
        .add(new Striped.SmallLazyStriped<Lock>(50, LOCK_SUPPLER))
        .add(new Striped.SmallLazyStriped<Lock>(64, LOCK_SUPPLER))
        .add(new Striped.LargeLazyStriped<Lock>(50, LOCK_SUPPLER))
        .add(new Striped.LargeLazyStriped<Lock>(64, LOCK_SUPPLER))
        .add(new Striped.SmallLazyStriped<Semaphore>(50, SEMAPHORE_SUPPLER))
        .add(new Striped.SmallLazyStriped<Semaphore>(64, SEMAPHORE_SUPPLER))
        .add(new Striped.LargeLazyStriped<Semaphore>(50, SEMAPHORE_SUPPLER))
        .add(new Striped.LargeLazyStriped<Semaphore>(64, SEMAPHORE_SUPPLER))
        .build();
  }

  private static Iterable<Striped<?>> allImplementations() {
    return concat(strongImplementations(), weakImplementations());
  }

  public void testNull() throws Exception {
    for (Striped<?> striped : allImplementations()) {
      new NullPointerTester().testAllPublicInstanceMethods(striped);
    }
  }

  public void testSizes() {
        assertTrue(Striped.lock(100).size() >= 100);
    assertTrue(Striped.lock(256).size() == 256);
    assertTrue(Striped.lazyWeakLock(100).size() >= 100);
    assertTrue(Striped.lazyWeakLock(256).size() == 256);
  }

  public void testWeakImplementations() {
    for (Striped<?> striped : weakImplementations()) {
      WeakReference<Object> weakRef = new WeakReference<>(striped.get(new Object()));
      GcFinalization.awaitClear(weakRef);
    }
  }

  public void testWeakReadWrite() {
    Striped<ReadWriteLock> striped = Striped.lazyWeakReadWriteLock(1000);
    Object key = new Object();
    Lock readLock = striped.get(key).readLock();
    WeakReference<Object> garbage = new WeakReference<>(new Object());
    GcFinalization.awaitClear(garbage);
    Lock writeLock = striped.get(key).writeLock();
    readLock.lock();
    assertFalse(writeLock.tryLock());
    readLock.unlock();
  }

  public void testStrongImplementations() {
    for (Striped<?> striped : strongImplementations()) {
      WeakReference<Object> weakRef = new WeakReference<>(striped.get(new Object()));
      WeakReference<Object> garbage = new WeakReference<>(new Object());
      GcFinalization.awaitClear(garbage);
      assertNotNull(weakRef.get());
    }
  }

  public void testMaximalWeakStripedLock() {
    Striped<Lock> stripedLock = Striped.lazyWeakLock(Integer.MAX_VALUE);
    for (int i = 0; i < 10000; i++) {
      stripedLock.get(new Object()).lock();
          }
  }

  public void testBulkGetReturnsSorted() {
    for (Striped<?> striped : allImplementations()) {
      Map<Object, Integer> indexByLock = Maps.newHashMap();
      for (int i = 0; i < striped.size(); i++) {
        indexByLock.put(striped.getAt(i), i);
      }

            for (int objectsNum = 1; objectsNum <= striped.size() * 2; objectsNum++) {
        Set<Object> objects = Sets.newHashSetWithExpectedSize(objectsNum);
        for (int i = 0; i < objectsNum; i++) {
          objects.add(new Object());
        }

        Iterable<?> locks = striped.bulkGet(objects);
        assertTrue(Ordering.natural().onResultOf(Functions.forMap(indexByLock)).isOrdered(locks));

                Iterable<?> locks2 = striped.bulkGet(objects);
        assertEquals(Lists.newArrayList(locks), Lists.newArrayList(locks2));
      }
    }
  }

  
  public void testBasicInvariants() {
    for (Striped<?> striped : allImplementations()) {
      assertBasicInvariants(striped);
    }
  }

  private static void assertBasicInvariants(Striped<?> striped) {
    Set<Object> observed = Sets.newIdentityHashSet();         for (int i = 0; i < striped.size(); i++) {
      Object object = striped.getAt(i);
      assertNotNull(object);
      assertSame(object, striped.getAt(i));       observed.add(object);
    }
    assertTrue("All stripes observed", observed.size() == striped.size());

        for (int i = 0; i < striped.size() * 100; i++) {
      assertTrue(observed.contains(striped.get(new Object())));
    }

    try {
      striped.getAt(-1);
      fail();
    } catch (RuntimeException expected) {
    }

    try {
      striped.getAt(striped.size());
      fail();
    } catch (RuntimeException expected) {
    }
  }

  public void testMaxSize() {
    for (Striped<?> striped :
        ImmutableList.of(
            Striped.lazyWeakLock(Integer.MAX_VALUE),
            Striped.lazyWeakSemaphore(Integer.MAX_VALUE, Integer.MAX_VALUE),
            Striped.lazyWeakReadWriteLock(Integer.MAX_VALUE))) {
      for (int i = 0; i < 3; i++) {
                Object unused = striped.getAt(Integer.MAX_VALUE - i);
      }
    }
  }
}
