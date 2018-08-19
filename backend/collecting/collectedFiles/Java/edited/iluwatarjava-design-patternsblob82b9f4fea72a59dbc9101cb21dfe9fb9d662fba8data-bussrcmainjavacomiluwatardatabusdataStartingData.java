

package com.iluwatar.databus.data;

import com.iluwatar.databus.AbstractDataType;
import com.iluwatar.databus.DataType;

import java.time.LocalDateTime;


public class StartingData extends AbstractDataType {

  private final LocalDateTime when;

  public StartingData(LocalDateTime when) {
    this.when = when;
  }

  public LocalDateTime getWhen() {
    return when;
  }

  public static DataType of(final LocalDateTime when) {
    return new StartingData(when);
  }
}
