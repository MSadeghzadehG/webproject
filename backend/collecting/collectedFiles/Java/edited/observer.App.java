
package com.iluwatar.observer;

import com.iluwatar.observer.generic.GHobbits;
import com.iluwatar.observer.generic.GOrcs;
import com.iluwatar.observer.generic.GWeather;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    Weather weather = new Weather();
    weather.addObserver(new Orcs());
    weather.addObserver(new Hobbits());

    weather.timePasses();
    weather.timePasses();
    weather.timePasses();
    weather.timePasses();

        LOGGER.info("--Running generic version--");
    GWeather gWeather = new GWeather();
    gWeather.addObserver(new GOrcs());
    gWeather.addObserver(new GHobbits());

    gWeather.timePasses();
    gWeather.timePasses();
    gWeather.timePasses();
    gWeather.timePasses();
  }
}
