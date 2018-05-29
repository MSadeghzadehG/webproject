

package com.iluwatar.databus;

import java.util.HashSet;
import java.util.Set;


public class DataBus {

  private static final DataBus INSTANCE = new DataBus();

  private final Set<Member> listeners = new HashSet<>();

  public static DataBus getInstance() {
    return INSTANCE;
  }

  
  public void subscribe(final Member member) {
    this.listeners.add(member);
  }

  
  public void unsubscribe(final Member member) {
    this.listeners.remove(member);
  }

  
  public void publish(final DataType event) {
    event.setDataBus(this);
    listeners.forEach(listener -> listener.accept(event));
  }
}
