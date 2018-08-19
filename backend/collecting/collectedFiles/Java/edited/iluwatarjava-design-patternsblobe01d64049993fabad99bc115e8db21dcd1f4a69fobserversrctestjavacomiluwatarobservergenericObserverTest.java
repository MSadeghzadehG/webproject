
package com.iluwatar.observer.generic;

import com.iluwatar.observer.WeatherType;
import com.iluwatar.observer.utils.InMemoryAppender;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.TestInstance;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.Collection;
import java.util.function.Supplier;

import static org.junit.jupiter.api.Assertions.assertEquals;


@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public abstract class ObserverTest<O extends Observer> {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  private final Supplier<O> factory;

  
  ObserverTest(final Supplier<O> factory) {
    this.factory = factory;
  }

  public abstract Collection<Object[]> dataProvider();

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testObserver(WeatherType weather, String response) {
    final O observer = this.factory.get();
    assertEquals(0, appender.getLogSize());

    observer.update(null, weather);
    assertEquals(response, appender.getLastMessage());
    assertEquals(1, appender.getLogSize());
  }

}
