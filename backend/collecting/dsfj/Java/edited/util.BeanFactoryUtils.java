
package com.alibaba.dubbo.config.spring.util;

import com.alibaba.dubbo.common.utils.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.config.ConfigurableBeanFactory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import static org.springframework.beans.factory.BeanFactoryUtils.beanNamesForTypeIncludingAncestors;
import static org.springframework.beans.factory.BeanFactoryUtils.beansOfTypeIncludingAncestors;


public class BeanFactoryUtils {


    
    public static <T> T getOptionalBean(ListableBeanFactory beanFactory, String beanName, Class<T> beanType) {

        String[] allBeanNames = beanNamesForTypeIncludingAncestors(beanFactory, beanType);

        if (!StringUtils.isContains(allBeanNames, beanName)) {
            return null;
        }

        Map<String, T> beansOfType = beansOfTypeIncludingAncestors(beanFactory, beanType);

        return beansOfType.get(beanName);

    }


    
    public static <T> List<T> getBeans(ListableBeanFactory beanFactory, String[] beanNames, Class<T> beanType) {

        String[] allBeanNames = beanNamesForTypeIncludingAncestors(beanFactory, beanType);

        List<T> beans = new ArrayList<T>(beanNames.length);

        for (String beanName : beanNames) {
            if (StringUtils.isContains(allBeanNames, beanName)) {
                beans.add(beanFactory.getBean(beanName, beanType));
            }
        }

        return Collections.unmodifiableList(beans);

    }

}
