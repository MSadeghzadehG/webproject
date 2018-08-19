

package com.iluwatar.featuretoggle;

import com.iluwatar.featuretoggle.pattern.Service;
import com.iluwatar.featuretoggle.pattern.propertiesversion.PropertiesFeatureToggleVersion;
import com.iluwatar.featuretoggle.pattern.tieredversion.TieredFeatureToggleVersion;
import com.iluwatar.featuretoggle.user.User;
import com.iluwatar.featuretoggle.user.UserGroup;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Properties;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    final Properties properties = new Properties();
    properties.put("enhancedWelcome", true);
    Service service = new PropertiesFeatureToggleVersion(properties);
    final String welcomeMessage = service.getWelcomeMessage(new User("Jamie No Code"));
    LOGGER.info(welcomeMessage);

    
    final Properties turnedOff = new Properties();
    turnedOff.put("enhancedWelcome", false);
    Service turnedOffService = new PropertiesFeatureToggleVersion(turnedOff);
    final String welcomeMessageturnedOff = turnedOffService.getWelcomeMessage(new User("Jamie No Code"));
    LOGGER.info(welcomeMessageturnedOff);

        
    Service service2 = new TieredFeatureToggleVersion();

    final User paidUser = new User("Jamie Coder");
    final User freeUser = new User("Alan Defect");

    UserGroup.addUserToPaidGroup(paidUser);
    UserGroup.addUserToFreeGroup(freeUser);

    final String welcomeMessagePaidUser = service2.getWelcomeMessage(paidUser);
    final String welcomeMessageFreeUser = service2.getWelcomeMessage(freeUser);
    LOGGER.info(welcomeMessageFreeUser);
    LOGGER.info(welcomeMessagePaidUser);
  }
}
