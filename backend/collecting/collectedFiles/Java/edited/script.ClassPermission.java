

package org.elasticsearch.script;

import java.security.BasicPermission;
import java.security.Permission;
import java.security.PermissionCollection;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Set;


public final class ClassPermission extends BasicPermission {
    public static final String STANDARD = "<<STANDARD>>";
    
        public static final Set<String> STANDARD_CLASSES = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(
                        java.lang.Boolean.class.getName(),
            java.lang.Byte.class.getName(),
            java.lang.Character.class.getName(),
            java.lang.Double.class.getName(),
            java.lang.Integer.class.getName(),
            java.lang.Long.class.getName(),
            java.lang.Math.class.getName(),
            java.lang.Object.class.getName(),
            java.lang.Short.class.getName(),
            java.lang.String.class.getName(),
            java.math.BigDecimal.class.getName(),
            java.util.ArrayList.class.getName(),
            java.util.Arrays.class.getName(),
            java.util.Date.class.getName(),
            java.util.HashMap.class.getName(),
            java.util.HashSet.class.getName(),
            java.util.Iterator.class.getName(),
            java.util.List.class.getName(),
            java.util.Map.class.getName(),
            java.util.Set.class.getName(),
            java.util.UUID.class.getName(),
                        org.joda.time.DateTime.class.getName(),
            org.joda.time.DateTimeUtils.class.getName(),
            org.joda.time.DateTimeZone.class.getName(),
            org.joda.time.Instant.class.getName(),
            org.joda.time.ReadableDateTime.class.getName(),
            org.joda.time.ReadableInstant.class.getName()
     )));

    
    public ClassPermission(String name) {
        super(name);
    }

    
    public ClassPermission(String name, String actions) {
        this(name);
    }

    @Override
    public boolean implies(Permission p) {
                if (p != null && p.getClass() == getClass()) {
            ClassPermission other = (ClassPermission) p;
            if (STANDARD.equals(getName()) && STANDARD_CLASSES.contains(other.getName())) {
                return true;
            }
        }
        return super.implies(p);
    }

    @Override
    public PermissionCollection newPermissionCollection() {
                PermissionCollection impl = super.newPermissionCollection();
        return new PermissionCollection() {
            @Override
            public void add(Permission permission) {
                if (permission instanceof ClassPermission && STANDARD.equals(permission.getName())) {
                    for (String clazz : STANDARD_CLASSES) {
                        impl.add(new ClassPermission(clazz));
                    }
                } else {
                    impl.add(permission);
                }
            }

            @Override
            public boolean implies(Permission permission) {
                return impl.implies(permission);
            }

            @Override
            public Enumeration<Permission> elements() {
                return impl.elements();
            }
        };
    }
}
