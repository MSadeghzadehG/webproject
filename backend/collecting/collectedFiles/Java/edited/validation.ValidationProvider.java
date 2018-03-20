
package com.alibaba.dubbo.examples.validation;

import org.springframework.context.support.ClassPathXmlApplicationContext;


public class ValidationProvider {

    public static void main(String[] args) throws Exception {
        System.setProperty("java.net.preferIPv4Stack", "true");
        String config = ValidationProvider.class.getPackage().getName().replace('.', '/') + "/validation-provider.xml";
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(config);
        context.start();
        System.in.read();
    }

}
