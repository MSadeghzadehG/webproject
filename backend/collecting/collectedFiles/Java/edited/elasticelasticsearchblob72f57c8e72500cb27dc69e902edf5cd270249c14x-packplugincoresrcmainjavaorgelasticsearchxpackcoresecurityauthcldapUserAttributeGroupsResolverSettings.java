
package org.elasticsearch.xpack.core.security.authc.ldap;

import org.elasticsearch.common.settings.Setting;

import java.util.Collections;
import java.util.Set;
import java.util.function.Function;

public final class UserAttributeGroupsResolverSettings {
    public static final Setting<String> ATTRIBUTE = new Setting<>("user_group_attribute", "memberOf",
            Function.identity(), Setting.Property.NodeScope);

    private UserAttributeGroupsResolverSettings() {}

    public static Set<Setting<?>> getSettings() {
        return Collections.singleton(ATTRIBUTE);
    }
}
