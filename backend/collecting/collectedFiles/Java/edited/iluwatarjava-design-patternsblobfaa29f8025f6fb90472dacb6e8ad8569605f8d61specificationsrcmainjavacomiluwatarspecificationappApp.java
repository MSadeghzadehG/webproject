
package com.iluwatar.specification.app;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import com.iluwatar.specification.creature.Creature;
import com.iluwatar.specification.creature.Dragon;
import com.iluwatar.specification.creature.Goblin;
import com.iluwatar.specification.creature.KillerBee;
import com.iluwatar.specification.creature.Octopus;
import com.iluwatar.specification.creature.Shark;
import com.iluwatar.specification.creature.Troll;
import com.iluwatar.specification.property.Color;
import com.iluwatar.specification.property.Movement;
import com.iluwatar.specification.selector.ColorSelector;
import com.iluwatar.specification.selector.MovementSelector;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {
  
  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {
        List<Creature> creatures =
        Arrays.asList(new Goblin(), new Octopus(), new Dragon(), new Shark(), new Troll(),
            new KillerBee());
        LOGGER.info("Find all walking creatures");
    List<Creature> walkingCreatures =
        creatures.stream().filter(new MovementSelector(Movement.WALKING))
            .collect(Collectors.toList());
    walkingCreatures.stream().forEach(c -> LOGGER.info(c.toString()));
        LOGGER.info("Find all dark creatures");
    List<Creature> darkCreatures =
        creatures.stream().filter(new ColorSelector(Color.DARK)).collect(Collectors.toList());
    darkCreatures.stream().forEach(c -> LOGGER.info(c.toString()));
        LOGGER.info("Find all red and flying creatures");
    List<Creature> redAndFlyingCreatures =
        creatures.stream()
            .filter(new ColorSelector(Color.RED).and(new MovementSelector(Movement.FLYING)))
            .collect(Collectors.toList());
    redAndFlyingCreatures.stream().forEach(c -> LOGGER.info(c.toString()));
  }
}
