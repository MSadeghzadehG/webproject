
package com.iluwatar.singleton;


public final class InitializingOnDemandHolderIdiom {

  
  private InitializingOnDemandHolderIdiom() {}

  
  public static InitializingOnDemandHolderIdiom getInstance() {
    return HelperHolder.INSTANCE;
  }

  
  private static class HelperHolder {
    private static final InitializingOnDemandHolderIdiom INSTANCE =
        new InitializingOnDemandHolderIdiom();
  }
}
