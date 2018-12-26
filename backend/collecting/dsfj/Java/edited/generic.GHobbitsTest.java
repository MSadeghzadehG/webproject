
package com.iluwatar.observer.generic;

import com.iluwatar.observer.WeatherType;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;


public class GHobbitsTest extends ObserverTest<GHobbits> {

  @Override
  public Collection<Object[]> dataProvider() {
    final List<Object[]> testData = new ArrayList<>();
    testData.add(new Object[]{WeatherType.SUNNY, "The happy hobbits bade in the warm sun."});
    testData.add(new Object[]{WeatherType.RAINY, "The hobbits look for cover from the rain."});
    testData.add(new Object[]{WeatherType.WINDY, "The hobbits hold their hats tightly in the windy weather."});
    testData.add(new Object[]{WeatherType.COLD, "The hobbits are shivering in the cold weather."});
    return testData;
  }

  
  public GHobbitsTest() {
    super(GHobbits::new);
  }

}
