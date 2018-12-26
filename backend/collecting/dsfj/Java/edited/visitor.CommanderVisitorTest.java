
package com.iluwatar.visitor;

import java.util.Optional;


public class CommanderVisitorTest extends VisitorTest<CommanderVisitor> {

  
  public CommanderVisitorTest() {
    super(
        new CommanderVisitor(),
        Optional.of("Good to see you commander"),
        Optional.empty(),
        Optional.empty()
    );
  }

}
