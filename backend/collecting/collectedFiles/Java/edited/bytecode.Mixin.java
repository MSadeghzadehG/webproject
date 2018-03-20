
package com.alibaba.dubbo.common.bytecode;

import com.alibaba.dubbo.common.utils.ClassHelper;
import com.alibaba.dubbo.common.utils.ReflectUtils;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;


public abstract class Mixin {
    private static final String PACKAGE_NAME = Mixin.class.getPackage().getName();
    private static AtomicLong MIXIN_CLASS_COUNTER = new AtomicLong(0);

    protected Mixin() {
    }

    
    public static Mixin mixin(Class<?>[] ics, Class<?> dc) {
        return mixin(ics, new Class[]{dc});
    }

    
    public static Mixin mixin(Class<?>[] ics, Class<?> dc, ClassLoader cl) {
        return mixin(ics, new Class[]{dc}, cl);
    }

    
    public static Mixin mixin(Class<?>[] ics, Class<?>[] dcs) {
        return mixin(ics, dcs, ClassHelper.getCallerClassLoader(Mixin.class));
    }

    
    public static Mixin mixin(Class<?>[] ics, Class<?>[] dcs, ClassLoader cl) {
        assertInterfaceArray(ics);

        long id = MIXIN_CLASS_COUNTER.getAndIncrement();
        String pkg = null;
        ClassGenerator ccp = null, ccm = null;
        try {
            ccp = ClassGenerator.newInstance(cl);

                        StringBuilder code = new StringBuilder();
            for (int i = 0; i < dcs.length; i++) {
                if (!Modifier.isPublic(dcs[i].getModifiers())) {
                    String npkg = dcs[i].getPackage().getName();
                    if (pkg == null) {
                        pkg = npkg;
                    } else {
                        if (!pkg.equals(npkg))
                            throw new IllegalArgumentException("non-public interfaces class from different packages");
                    }
                }

                ccp.addField("private " + dcs[i].getName() + " d" + i + ";");

                code.append("d").append(i).append(" = (").append(dcs[i].getName()).append(")$1[").append(i).append("];\n");
                if (MixinAware.class.isAssignableFrom(dcs[i]))
                    code.append("d").append(i).append(".setMixinInstance(this);\n");
            }
            ccp.addConstructor(Modifier.PUBLIC, new Class<?>[]{Object[].class}, code.toString());

                        Set<String> worked = new HashSet<String>();
            for (int i = 0; i < ics.length; i++) {
                if (!Modifier.isPublic(ics[i].getModifiers())) {
                    String npkg = ics[i].getPackage().getName();
                    if (pkg == null) {
                        pkg = npkg;
                    } else {
                        if (!pkg.equals(npkg))
                            throw new IllegalArgumentException("non-public delegate class from different packages");
                    }
                }

                ccp.addInterface(ics[i]);

                for (Method method : ics[i].getMethods()) {
                    if ("java.lang.Object".equals(method.getDeclaringClass().getName()))
                        continue;

                    String desc = ReflectUtils.getDesc(method);
                    if (worked.contains(desc))
                        continue;
                    worked.add(desc);

                    int ix = findMethod(dcs, desc);
                    if (ix < 0)
                        throw new RuntimeException("Missing method [" + desc + "] implement.");

                    Class<?> rt = method.getReturnType();
                    String mn = method.getName();
                    if (Void.TYPE.equals(rt))
                        ccp.addMethod(mn, method.getModifiers(), rt, method.getParameterTypes(), method.getExceptionTypes(),
                                "d" + ix + "." + mn + "($$);");
                    else
                        ccp.addMethod(mn, method.getModifiers(), rt, method.getParameterTypes(), method.getExceptionTypes(),
                                "return ($r)d" + ix + "." + mn + "($$);");
                }
            }

            if (pkg == null)
                pkg = PACKAGE_NAME;

                        String micn = pkg + ".mixin" + id;
            ccp.setClassName(micn);
            ccp.toClass();

                        String fcn = Mixin.class.getName() + id;
            ccm = ClassGenerator.newInstance(cl);
            ccm.setClassName(fcn);
            ccm.addDefaultConstructor();
            ccm.setSuperClass(Mixin.class.getName());
            ccm.addMethod("public Object newInstance(Object[] delegates){ return new " + micn + "($1); }");
            Class<?> mixin = ccm.toClass();
            return (Mixin) mixin.newInstance();
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
        } finally {
                        if (ccp != null)
                ccp.release();
            if (ccm != null)
                ccm.release();
        }
    }

    private static int findMethod(Class<?>[] dcs, String desc) {
        Class<?> cl;
        Method[] methods;
        for (int i = 0; i < dcs.length; i++) {
            cl = dcs[i];
            methods = cl.getMethods();
            for (Method method : methods) {
                if (desc.equals(ReflectUtils.getDesc(method)))
                    return i;
            }
        }
        return -1;
    }

    private static void assertInterfaceArray(Class<?>[] ics) {
        for (int i = 0; i < ics.length; i++)
            if (!ics[i].isInterface())
                throw new RuntimeException("Class " + ics[i].getName() + " is not a interface.");
    }

    
    abstract public Object newInstance(Object[] ds);

    public static interface MixinAware {
        void setMixinInstance(Object instance);
    }
}