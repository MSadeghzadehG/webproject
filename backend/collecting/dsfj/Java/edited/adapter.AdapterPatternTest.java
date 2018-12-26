
package com.iluwatar.adapter;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.HashMap;
import java.util.Map;

import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.verify;


public class AdapterPatternTest {

  private Map<String, Object> beans;

  private static final String FISHING_BEAN = "fisher";

  private static final String ROWING_BEAN = "captain";

  
  @BeforeEach
  public void setup() {
    beans = new HashMap<>();

    FishingBoatAdapter fishingBoatAdapter = spy(new FishingBoatAdapter());
    beans.put(FISHING_BEAN, fishingBoatAdapter);

    Captain captain = new Captain();
    captain.setRowingBoat((FishingBoatAdapter) beans.get(FISHING_BEAN));
    beans.put(ROWING_BEAN, captain);
  }

  
  @Test
  public void testAdapter() {
    Captain captain = (Captain) beans.get(ROWING_BEAN);

        captain.row();

        RowingBoat adapter = (RowingBoat) beans.get(FISHING_BEAN);
    verify(adapter).row();
  }
}
