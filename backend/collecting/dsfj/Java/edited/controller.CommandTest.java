
package com.iluwatar.front.controller;

import com.iluwatar.front.controller.utils.InMemoryAppender;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class CommandTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  static List<Object[]> dataProvider() {
    final List<Object[]> parameters = new ArrayList<>();
    parameters.add(new Object[]{"Archer", "Displaying archers"});
    parameters.add(new Object[]{"Catapult", "Displaying catapults"});
    parameters.add(new Object[]{"NonExistentCommand", "Error 500"});
    return parameters;
  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testDisplay(String request, String displayMessage) {
    final FrontController frontController = new FrontController();
    assertEquals(0, appender.getLogSize());
    frontController.handleRequest(request);
    assertEquals(displayMessage, appender.getLastMessage());
    assertEquals(1, appender.getLogSize());
  }

}
