
package com.iluwatar.mediator;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.function.Supplier;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;


public class PartyMemberTest {

  static Collection<Supplier<PartyMember>[]> dataProvider() {
    return Arrays.asList(
            new Supplier[]{Hobbit::new},
            new Supplier[]{Hunter::new},
            new Supplier[]{Rogue::new},
            new Supplier[]{Wizard::new}
    );
  }

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(PartyMemberBase.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testPartyAction(Supplier<PartyMember> memberSupplier) {
    final PartyMember member = memberSupplier.get();

    for (final Action action : Action.values()) {
      member.partyAction(action);
      assertEquals(member.toString() + " " + action.getDescription(), appender.getLastMessage());
    }

    assertEquals(Action.values().length, appender.getLogSize());
  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testAct(Supplier<PartyMember> memberSupplier) {
    final PartyMember member = memberSupplier.get();

    member.act(Action.GOLD);
    assertEquals(0, appender.getLogSize());

    final Party party = mock(Party.class);
    member.joinedParty(party);
    assertEquals(member.toString() + " joins the party", appender.getLastMessage());

    for (final Action action : Action.values()) {
      member.act(action);
      assertEquals(member.toString() + " " + action.toString(), appender.getLastMessage());
      verify(party).act(member, action);
    }

    assertEquals(Action.values().length + 1, appender.getLogSize());
  }

  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testToString(Supplier<PartyMember> memberSupplier) throws Exception {
    final PartyMember member = memberSupplier.get();
    final Class<? extends PartyMember> memberClass = member.getClass();
    assertEquals(memberClass.getSimpleName(), member.toString());
  }

  private class InMemoryAppender extends AppenderBase<ILoggingEvent> {
    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender(Class clazz) {
      ((Logger) LoggerFactory.getLogger(clazz)).addAppender(this);
      start();
    }

    @Override
    protected void append(ILoggingEvent eventObject) {
      log.add(eventObject);
    }

    public int getLogSize() {
      return log.size();
    }

    public String getLastMessage() {
      return log.get(log.size() - 1).getFormattedMessage();
    }
  }


}
