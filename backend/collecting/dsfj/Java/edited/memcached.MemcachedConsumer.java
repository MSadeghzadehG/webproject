
package com.alibaba.dubbo.examples.memcached;

import org.springframework.context.support.ClassPathXmlApplicationContext;

import java.util.Map;


public class MemcachedConsumer {

    @SuppressWarnings("unchecked")
    public static void main(String[] args) throws Exception {
        String config = MemcachedConsumer.class.getPackage().getName().replace('.', '/') + "/memcached-consumer.xml";
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(config);
        context.start();
        Map<String, Object> cache = (Map<String, Object>) context.getBean("cache");
        cache.remove("hello");
        Object value = cache.get("hello");
        System.out.println(value);
        if (value != null) {
            throw new IllegalStateException(value + " != null");
        }
        cache.put("hello", "world");
        value = cache.get("hello");
        System.out.println(value);
        if (!"world".equals(value)) {
            throw new IllegalStateException(value + " != world");
        }
        System.in.read();
    }

}
