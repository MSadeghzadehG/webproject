
package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.spi.Element;
import org.elasticsearch.common.inject.spi.Elements;
import org.elasticsearch.common.inject.spi.InstanceBinding;
import org.elasticsearch.common.inject.spi.LinkedKeyBinding;
import org.elasticsearch.common.inject.spi.ProviderInstanceBinding;
import org.elasticsearch.common.inject.spi.ProviderLookup;
import org.elasticsearch.common.inject.spi.UntargettedBinding;
import org.elasticsearch.test.ESTestCase;

import java.lang.annotation.Annotation;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;


public abstract class ModuleTestCase extends ESTestCase {

    
    public void assertBinding(Module module, Class to, Class clazz) {
        List<Element> elements = Elements.getElements(module);
        for (Element element : elements) {
            if (element instanceof LinkedKeyBinding) {
                LinkedKeyBinding binding = (LinkedKeyBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    assertSame(clazz, binding.getLinkedKey().getTypeLiteral().getType());
                    return;
                }
            } else if (element instanceof UntargettedBinding) {
                UntargettedBinding binding = (UntargettedBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    assertSame(clazz, to);
                    return;
                }
            }
        }
        StringBuilder s = new StringBuilder();
        for (Element element : elements) {
            s.append(element).append("\n");
        }
        fail("Did not find any binding to " + to.getName() + ". Found these bindings:\n" + s);
    }

    
    public void assertNotBound(Module module, Class clazz) {
        List<Element> elements = Elements.getElements(module);
        for (Element element : elements) {
            if (element instanceof LinkedKeyBinding) {
                LinkedKeyBinding binding = (LinkedKeyBinding) element;
                if (clazz.equals(binding.getKey().getTypeLiteral().getType())) {
                    fail("Found binding for " + clazz.getName() + " to " + binding.getKey().getTypeLiteral().getType().getTypeName());
                }
            } else if (element instanceof UntargettedBinding) {
                UntargettedBinding binding = (UntargettedBinding) element;
                if (clazz.equals(binding.getKey().getTypeLiteral().getType())) {
                    fail("Found binding for " + clazz.getName());
                }
            }
        }
    }

    
    public void assertBindingFailure(Module module, String... msgs) {
        try {
            List<Element> elements = Elements.getElements(module);
            StringBuilder s = new StringBuilder();
            for (Element element : elements) {
                s.append(element).append("\n");
            }
            fail("Expected exception from configuring module. Found these bindings:\n" + s);
        } catch (IllegalArgumentException e) {
            for (String msg : msgs) {
                assertTrue(e.getMessage() + " didn't contain: " + msg, e.getMessage().contains(msg));
            }
        }
    }

    
    public void assertMapMultiBinding(Module module, Class to, Class theClass) {
        List<Element> elements = Elements.getElements(module);
        Set<Type> bindings = new HashSet<>();
        boolean providerFound = false;
        for (Element element : elements) {
            if (element instanceof LinkedKeyBinding) {
                LinkedKeyBinding binding = (LinkedKeyBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    bindings.add(binding.getLinkedKey().getTypeLiteral().getType());
                }
            } else if (element instanceof ProviderInstanceBinding) {
                ProviderInstanceBinding binding = (ProviderInstanceBinding) element;
                String setType = binding.getKey().getTypeLiteral().getType().toString();
                if (setType.equals("java.util.Map<java.lang.String, " + to.getName() + ">")) {
                    providerFound = true;
                }
            }
        }

        if (bindings.contains(theClass) == false) {
            fail("Expected to find " + theClass.getName() + " as binding to " + to.getName() + ", found these classes:\n" + bindings);
        }
        assertTrue("Did not find provider for map of " + to.getName(), providerFound);
    }


    
    public void assertSetMultiBinding(Module module, Class to, Class... classes) {
        List<Element> elements = Elements.getElements(module);
        Set<Type> bindings = new HashSet<>();
        boolean providerFound = false;
        for (Element element : elements) {
            if (element instanceof LinkedKeyBinding) {
                LinkedKeyBinding binding = (LinkedKeyBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    bindings.add(binding.getLinkedKey().getTypeLiteral().getType());
                }
            } else if (element instanceof ProviderInstanceBinding) {
                ProviderInstanceBinding binding = (ProviderInstanceBinding) element;
                String setType = binding.getKey().getTypeLiteral().getType().toString();
                if (setType.equals("java.util.Set<" + to.getName() + ">")) {
                    providerFound = true;
                }
            }
        }

        for (Class clazz : classes) {
            if (bindings.contains(clazz) == false) {
                fail("Expected to find " + clazz.getName() + " as set binding to " + to.getName() + ", found these classes:\n" + bindings);
            }
        }
        assertTrue("Did not find provider for set of " + to.getName(), providerFound);
    }

    
    public <T> void assertInstanceBinding(Module module, Class<T> to, Predicate<T> tester) {
        assertInstanceBindingWithAnnotation(module, to, tester, null);
    }

    
    public static <T> T bindAndGetInstance(Module module, Class<T> to) {
        List<Element> elements = Elements.getElements(module);
        for (Element element : elements) {
            if (element instanceof InstanceBinding) {
                InstanceBinding binding = (InstanceBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    return to.cast(binding.getInstance());
                }
            } else  if (element instanceof ProviderInstanceBinding) {
                ProviderInstanceBinding binding = (ProviderInstanceBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    return to.cast(binding.getProviderInstance().get());
                }
            }
        }
        fail("can't get instance for class " + to);
        return null;     }

    
    public <T> void assertInstanceBindingWithAnnotation(Module module, Class<T> to, Predicate<T> tester, Class<? extends Annotation> annotation) {
        List<Element> elements = Elements.getElements(module);
        for (Element element : elements) {
            if (element instanceof InstanceBinding) {
                InstanceBinding binding = (InstanceBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    if (annotation == null || annotation.equals(binding.getKey().getAnnotationType())) {
                        assertTrue(tester.test(to.cast(binding.getInstance())));
                        return;
                    }
                }
            } else  if (element instanceof ProviderInstanceBinding) {
                ProviderInstanceBinding binding = (ProviderInstanceBinding) element;
                if (to.equals(binding.getKey().getTypeLiteral().getType())) {
                    assertTrue(tester.test(to.cast(binding.getProviderInstance().get())));
                    return;
                }
            }
        }
        StringBuilder s = new StringBuilder();
        for (Element element : elements) {
            s.append(element).append("\n");
        }
        fail("Did not find any instance binding to " + to.getName() + ". Found these bindings:\n" + s);
    }

    
    @SuppressWarnings("unchecked")
    public <K, V> void assertMapInstanceBinding(Module module, Class<K> keyType, Class<V> valueType, Map<K, V> expected) throws Exception {
                Map<K, Key> keys = new HashMap<>();
        Map<Key, V> values = new HashMap<>();
        List<Element> elements = Elements.getElements(module);
        for (Element element : elements) {
            if (element instanceof InstanceBinding) {
                InstanceBinding binding = (InstanceBinding) element;
                if (binding.getKey().getRawType().equals(valueType)) {
                    values.put(binding.getKey(), (V) binding.getInstance());
                } else if (binding.getInstance() instanceof Map.Entry) {
                    Map.Entry entry = (Map.Entry) binding.getInstance();
                    Object key = entry.getKey();
                    Object providerValue = entry.getValue();
                    if (key.getClass().equals(keyType) && providerValue instanceof ProviderLookup.ProviderImpl) {
                        ProviderLookup.ProviderImpl provider = (ProviderLookup.ProviderImpl) providerValue;
                        keys.put((K) key, provider.getKey());
                    }
                }
            }
        }
        for (Map.Entry<K, V> entry : expected.entrySet()) {
            Key valueKey = keys.get(entry.getKey());
            assertNotNull("Could not find binding for key [" + entry.getKey() + "], found these keys:\n" + keys.keySet(), valueKey);
            V value = values.get(valueKey);
            assertNotNull("Could not find value for instance key [" + valueKey + "], found these bindings:\n" + elements);
            assertEquals(entry.getValue(), value);
        }
    }
}
