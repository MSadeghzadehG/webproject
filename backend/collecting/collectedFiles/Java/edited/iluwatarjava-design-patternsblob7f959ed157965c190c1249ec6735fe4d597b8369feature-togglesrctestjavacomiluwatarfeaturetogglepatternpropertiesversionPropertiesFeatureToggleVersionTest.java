

package com.iluwatar.featuretoggle.pattern.propertiesversion;

import com.iluwatar.featuretoggle.pattern.Service;
import com.iluwatar.featuretoggle.user.User;
import org.junit.jupiter.api.Test;

import java.util.Properties;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class PropertiesFeatureToggleVersionTest {

  @Test
  public void testNullPropertiesPassed() throws Exception {
    assertThrows(IllegalArgumentException.class, () -> {
      new PropertiesFeatureToggleVersion(null);
    });
  }

  @Test
  public void testNonBooleanProperty() throws Exception {
    assertThrows(IllegalArgumentException.class, () -> {
      final Properties properties = new Properties();
      properties.setProperty("enhancedWelcome", "Something");
      new PropertiesFeatureToggleVersion(properties);
    });
  }

  @Test
  public void testFeatureTurnedOn() throws Exception {
    final Properties properties = new Properties();
    properties.put("enhancedWelcome", true);
    Service service = new PropertiesFeatureToggleVersion(properties);
    assertTrue(service.isEnhanced());
    final String welcomeMessage = service.getWelcomeMessage(new User("Jamie No Code"));
    assertEquals("Welcome Jamie No Code. You're using the enhanced welcome message.", welcomeMessage);
  }

  @Test
  public void testFeatureTurnedOff() throws Exception {
    final Properties properties = new Properties();
    properties.put("enhancedWelcome", false);
    Service service = new PropertiesFeatureToggleVersion(properties);
    assertFalse(service.isEnhanced());
    final String welcomeMessage = service.getWelcomeMessage(new User("Jamie No Code"));
    assertEquals("Welcome to the application.", welcomeMessage);
  }
}
