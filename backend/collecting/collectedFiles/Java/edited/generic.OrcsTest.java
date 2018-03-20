
package com.iluwatar.observer.generic;

import com.iluwatar.observer.WeatherType;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;


public class OrcsTest extends ObserverTest<GOrcs> {

  @Override
  public Collection<Object[]> dataProvider() {
    final List<Object[]> testData = new ArrayList<>();
    testData.add(new Object[]{WeatherType.SUNNY, "The sun hurts the orcs' eyes."});
    testData.add(new Object[]{WeatherType.RAINY, "The orcs are dripping wet."});
    testData.add(new Object[]{WeatherType.WINDY, "The orc smell almost vanishes in the wind."});
    testData.add(new Object[]{WeatherType.COLD, "The orcs are freezing cold."});
    return testData;
  }

  
  public OrcsTest() {
    super(GOrcs::new);
  }

}
