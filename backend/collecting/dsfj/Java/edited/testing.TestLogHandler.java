

package com.google.common.testing;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.logging.Handler;
import java.util.logging.LogRecord;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtCompatible
public class TestLogHandler extends Handler {
  
  private final List<LogRecord> list = new ArrayList<>();

  
  @Override
  public synchronized void publish(@NullableDecl LogRecord record) {
    list.add(record);
  }

  @Override
  public void flush() {}

  @Override
  public void close() {}

  public synchronized void clear() {
    list.clear();
  }

  
  
  public synchronized List<LogRecord> getStoredLogRecords() {
    List<LogRecord> result = new ArrayList<>(list);
    return Collections.unmodifiableList(result);
  }
}
