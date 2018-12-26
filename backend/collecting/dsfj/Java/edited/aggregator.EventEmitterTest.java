
package com.iluwatar.event.aggregator;

import org.junit.jupiter.api.Test;

import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;

import java.util.Objects;
import java.util.function.Function;
import java.util.function.Supplier;


public abstract class EventEmitterTest<E extends EventEmitter> {

  
  private final Function<EventObserver, E> factoryWithDefaultObserver;

  
  private final Supplier<E> factoryWithoutDefaultObserver;

  
  private final Weekday specialDay;

  
  private final Event event;

  
  EventEmitterTest(final Weekday specialDay, final Event event,
                   final Function<EventObserver, E> factoryWithDefaultObserver,
                   final Supplier<E> factoryWithoutDefaultObserver) {

    this.specialDay = specialDay;
    this.event = event;
    this.factoryWithDefaultObserver = Objects.requireNonNull(factoryWithDefaultObserver);
    this.factoryWithoutDefaultObserver = Objects.requireNonNull(factoryWithoutDefaultObserver);
  }

  
  @Test
  public void testAllDays() {
    testAllDaysWithoutDefaultObserver(specialDay, event);
    testAllDaysWithDefaultObserver(specialDay, event);
  }

  
  private void testAllDays(final Weekday specialDay, final Event event, final E emitter,
                           final EventObserver... observers) {

    for (final Weekday weekday : Weekday.values()) {
            emitter.timePasses(weekday);

      if (weekday == specialDay) {
                for (final EventObserver observer : observers) {
          verify(observer, times(1)).onEvent(eq(event));
        }
      } else {
                verifyZeroInteractions(observers);
      }
    }

        verifyNoMoreInteractions(observers);
  }

  
  private void testAllDaysWithoutDefaultObserver(final Weekday specialDay, final Event event) {
    final EventObserver observer1 = mock(EventObserver.class);
    final EventObserver observer2 = mock(EventObserver.class);

    final E emitter = this.factoryWithoutDefaultObserver.get();
    emitter.registerObserver(observer1);
    emitter.registerObserver(observer2);

    testAllDays(specialDay, event, emitter, observer1, observer2);
  }

  
  private void testAllDaysWithDefaultObserver(final Weekday specialDay, final Event event) {
    final EventObserver defaultObserver = mock(EventObserver.class);
    final EventObserver observer1 = mock(EventObserver.class);
    final EventObserver observer2 = mock(EventObserver.class);

    final E emitter = this.factoryWithDefaultObserver.apply(defaultObserver);
    emitter.registerObserver(observer1);
    emitter.registerObserver(observer2);

    testAllDays(specialDay, event, emitter, defaultObserver, observer1, observer2);
  }

}
