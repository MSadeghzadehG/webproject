
package com.iluwatar.object.pool;

import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.List;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTimeout;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class OliphauntPoolTest {

  
  @Test
  public void testSubsequentCheckinCheckout() {
    assertTimeout(ofMillis(5000), () -> {
      final OliphauntPool pool = new OliphauntPool();
      assertEquals(pool.toString(), "Pool available=0 inUse=0");

      final Oliphaunt expectedOliphaunt = pool.checkOut();
      assertEquals(pool.toString(), "Pool available=0 inUse=1");

      pool.checkIn(expectedOliphaunt);
      assertEquals(pool.toString(), "Pool available=1 inUse=0");

      for (int i = 0; i < 100; i++) {
        final Oliphaunt oliphaunt = pool.checkOut();
        assertEquals(pool.toString(), "Pool available=0 inUse=1");
        assertSame(expectedOliphaunt, oliphaunt);
        assertEquals(expectedOliphaunt.getId(), oliphaunt.getId());
        assertEquals(expectedOliphaunt.toString(), oliphaunt.toString());

        pool.checkIn(oliphaunt);
        assertEquals(pool.toString(), "Pool available=1 inUse=0");
      }
    });
  }

  
  @Test
  public void testConcurrentCheckinCheckout() {
    assertTimeout(ofMillis(5000), () -> {
      final OliphauntPool pool = new OliphauntPool();
      assertEquals(pool.toString(), "Pool available=0 inUse=0");

      final Oliphaunt firstOliphaunt = pool.checkOut();
      assertEquals(pool.toString(), "Pool available=0 inUse=1");

      final Oliphaunt secondOliphaunt = pool.checkOut();
      assertEquals(pool.toString(), "Pool available=0 inUse=2");

      assertNotSame(firstOliphaunt, secondOliphaunt);
      assertEquals(firstOliphaunt.getId() + 1, secondOliphaunt.getId());

            pool.checkIn(secondOliphaunt);
      assertEquals(pool.toString(), "Pool available=1 inUse=1");

      final Oliphaunt oliphaunt3 = pool.checkOut();
      assertEquals(pool.toString(), "Pool available=0 inUse=2");
      assertSame(secondOliphaunt, oliphaunt3);

            pool.checkIn(firstOliphaunt);
      assertEquals(pool.toString(), "Pool available=1 inUse=1");

      final Oliphaunt oliphaunt4 = pool.checkOut();
      assertEquals(pool.toString(), "Pool available=0 inUse=2");
      assertSame(firstOliphaunt, oliphaunt4);

            pool.checkIn(firstOliphaunt);
      assertEquals(pool.toString(), "Pool available=1 inUse=1");

      pool.checkIn(secondOliphaunt);
      assertEquals(pool.toString(), "Pool available=2 inUse=0");

                  final List<Oliphaunt> oliphaunts = Arrays.asList(pool.checkOut(), pool.checkOut());
      assertEquals(pool.toString(), "Pool available=0 inUse=2");
      assertTrue(oliphaunts.contains(firstOliphaunt));
      assertTrue(oliphaunts.contains(secondOliphaunt));
    });
  }


}