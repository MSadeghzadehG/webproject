
package com.iluwatar.visitor;

import java.util.Optional;


public class SergeantVisitorTest extends VisitorTest<SergeantVisitor> {

  
  public SergeantVisitorTest() {
    super(
        new SergeantVisitor(),
        Optional.empty(),
        Optional.of("Hello sergeant"),
        Optional.empty()
    );
  }

}
