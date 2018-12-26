

package com.iluwatar.mute;

import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;

import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class MuteTest {

  private static final Logger LOGGER = LoggerFactory.getLogger(MuteTest.class);

  private static final String MESSAGE = "should not occur";

  @Test
  public void muteShouldRunTheCheckedRunnableAndNotThrowAnyExceptionIfCheckedRunnableDoesNotThrowAnyException() {
    Mute.mute(() -> methodNotThrowingAnyException());
  }

  @Test
  public void muteShouldRethrowUnexpectedExceptionAsAssertionError() throws Exception {
    assertThrows(AssertionError.class, () -> {
      Mute.mute(() -> methodThrowingException());
    });
  }

  @Test
  public void loggedMuteShouldRunTheCheckedRunnableAndNotThrowAnyExceptionIfCheckedRunnableDoesNotThrowAnyException() {
    Mute.loggedMute(() -> methodNotThrowingAnyException());
  }

  @Test
  public void loggedMuteShouldLogExceptionTraceBeforeSwallowingIt() throws IOException {
    ByteArrayOutputStream stream = new ByteArrayOutputStream();
    System.setErr(new PrintStream(stream));

    Mute.loggedMute(() -> methodThrowingException());

    assertTrue(new String(stream.toByteArray()).contains(MESSAGE));
  }


  private void methodNotThrowingAnyException() {
    LOGGER.info("Executed successfully");
  }

  private void methodThrowingException() throws Exception {
    throw new Exception(MESSAGE);
  }
}
