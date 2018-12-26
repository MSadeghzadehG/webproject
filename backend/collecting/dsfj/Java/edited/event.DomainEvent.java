
package com.iluwatar.event.sourcing.event;

import java.io.Serializable;


public abstract class DomainEvent implements Serializable {

  private final long sequenceId;
  private final long createdTime;
  private final String eventClassName;
  private boolean realTime = true;

  
  public DomainEvent(long sequenceId, long createdTime, String eventClassName) {
    this.sequenceId = sequenceId;
    this.createdTime = createdTime;
    this.eventClassName = eventClassName;
  }

  
  public long getSequenceId() {
    return sequenceId;
  }

  
  public long getCreatedTime() {
    return createdTime;
  }

  
  public boolean isRealTime() {
    return realTime;
  }

  
  public void setRealTime(boolean realTime) {
    this.realTime = realTime;
  }

  
  public abstract void process();

  
  public String getEventClassName() {
    return eventClassName;
  }
}
