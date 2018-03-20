


package com.iluwatar.databus;

import java.util.function.Consumer;


public interface Member extends Consumer<DataType> {

  void accept(DataType event);
}
