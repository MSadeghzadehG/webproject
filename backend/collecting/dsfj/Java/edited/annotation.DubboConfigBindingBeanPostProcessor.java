
package com.alibaba.dubbo.config.spring.beans.factory.annotation;

import com.alibaba.dubbo.common.utils.Assert;
import com.alibaba.dubbo.config.spring.context.annotation.DubboConfigBindingRegistrar;
import com.alibaba.dubbo.config.spring.context.annotation.EnableDubboConfigBinding;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.BeansException;
import org.springframework.beans.PropertyValues;
import org.springframework.beans.factory.config.BeanPostProcessor;
import org.springframework.validation.DataBinder;

import java.util.Arrays;


public class DubboConfigBindingBeanPostProcessor implements BeanPostProcessor {

    private final Log log = LogFactory.getLog(getClass());

    
    private final String beanName;

    
    private final PropertyValues propertyValues;


    
    public DubboConfigBindingBeanPostProcessor(String beanName, PropertyValues propertyValues) {
        Assert.notNull(beanName, "The name of bean must not be null");
        Assert.notNull(propertyValues, "The PropertyValues of bean must not be null");
        this.beanName = beanName;
        this.propertyValues = propertyValues;
    }

    @Override
    public Object postProcessBeforeInitialization(Object bean, String beanName) throws BeansException {

        if (beanName.equals(this.beanName)) {
            DataBinder dataBinder = new DataBinder(bean);
                        dataBinder.setIgnoreInvalidFields(true);
            dataBinder.bind(propertyValues);
            if (log.isInfoEnabled()) {
                log.info("The properties of bean [name : " + beanName + "] have been binding by values : "
                        + Arrays.asList(propertyValues.getPropertyValues()));
            }
        }

        return bean;

    }


    @Override
    public Object postProcessAfterInitialization(Object bean, String beanName) throws BeansException {
        return bean;
    }

}
