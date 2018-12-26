
package com.iluwatar.nullobject;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class TreeTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  private static final Node TREE_ROOT;

  static {
    final NodeImpl level1B = new NodeImpl("level1_b", NullNode.getInstance(), NullNode.getInstance());
    final NodeImpl level2B = new NodeImpl("level2_b", NullNode.getInstance(), NullNode.getInstance());
    final NodeImpl level3A = new NodeImpl("level3_a", NullNode.getInstance(), NullNode.getInstance());
    final NodeImpl level3B = new NodeImpl("level3_b", NullNode.getInstance(), NullNode.getInstance());
    final NodeImpl level2A = new NodeImpl("level2_a", level3A, level3B);
    final NodeImpl level1A = new NodeImpl("level1_a", level2A, level2B);
    TREE_ROOT = new NodeImpl("root", level1A, level1B);
  }

  
  @Test
  public void testTreeSize() {
    assertEquals(7, TREE_ROOT.getTreeSize());
  }

  
  @Test
  public void testWalk() {
    TREE_ROOT.walk();

    assertTrue(appender.logContains("root"));
    assertTrue(appender.logContains("level1_a"));
    assertTrue(appender.logContains("level2_a"));
    assertTrue(appender.logContains("level3_a"));
    assertTrue(appender.logContains("level3_b"));
    assertTrue(appender.logContains("level2_b"));
    assertTrue(appender.logContains("level1_b"));
    assertEquals(7, appender.getLogSize());
  }

  @Test
  public void testGetLeft() throws Exception {
    final Node level1 = TREE_ROOT.getLeft();
    assertNotNull(level1);
    assertEquals("level1_a", level1.getName());
    assertEquals(5, level1.getTreeSize());

    final Node level2 = level1.getLeft();
    assertNotNull(level2);
    assertEquals("level2_a", level2.getName());
    assertEquals(3, level2.getTreeSize());

    final Node level3 = level2.getLeft();
    assertNotNull(level3);
    assertEquals("level3_a", level3.getName());
    assertEquals(1, level3.getTreeSize());
    assertSame(NullNode.getInstance(), level3.getRight());
    assertSame(NullNode.getInstance(), level3.getLeft());
  }

  @Test
  public void testGetRight() throws Exception {
    final Node level1 = TREE_ROOT.getRight();
    assertNotNull(level1);
    assertEquals("level1_b", level1.getName());
    assertEquals(1, level1.getTreeSize());
    assertSame(NullNode.getInstance(), level1.getRight());
    assertSame(NullNode.getInstance(), level1.getLeft());
  }

  private class InMemoryAppender extends AppenderBase<ILoggingEvent> {
    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender() {
      ((Logger) LoggerFactory.getLogger("root")).addAppender(this);
      start();
    }

    @Override
    protected void append(ILoggingEvent eventObject) {
      log.add(eventObject);
    }

    public boolean logContains(String message) {
      return log.stream().anyMatch(event -> event.getMessage().equals(message));
    }

    public int getLogSize() {
      return log.size();
    }
  }

}
