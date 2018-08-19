
package com.iluwatar.factorykit;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;
import java.util.function.Supplier;


public interface WeaponFactory {

  
  Weapon create(WeaponType name);

  
  static WeaponFactory factory(Consumer<Builder> consumer) {
    Map<WeaponType, Supplier<Weapon>> map = new HashMap<>();
    consumer.accept(map::put);
    return name -> map.get(name).get();
  }
}
