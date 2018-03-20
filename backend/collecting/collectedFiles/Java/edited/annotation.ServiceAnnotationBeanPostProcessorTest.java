
package com.alibaba.dubbo.config.spring.beans.factory.annotation;

import com.alibaba.dubbo.config.spring.ServiceBean;
import com.alibaba.dubbo.config.spring.api.HelloService;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.ImportResource;
import org.springframework.context.annotation.PropertySource;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.TestPropertySource;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Map;


@RunWith(SpringRunner.class)
@ContextConfiguration(
        classes = {ServiceAnnotationBeanPostProcessorTest.TestConfiguration.class})
@TestPropertySource(properties = {
        "package1 = com.alibaba.dubbo.config.spring.context.annotation",
        "packagesToScan = ${package1}"
})
public class ServiceAnnotationBeanPostProcessorTest {

    @Autowired
    private ConfigurableListableBeanFactory beanFactory;

    @Test
    public void test() {

        Map<String, HelloService> helloServicesMap = beanFactory.getBeansOfType(HelloService.class);

        Assert.assertEquals(2, helloServicesMap.size());

        Map<String, ServiceBean> serviceBeansMap = beanFactory.getBeansOfType(ServiceBean.class);

        Assert.assertEquals(3, serviceBeansMap.size());

        Map<String, ServiceAnnotationBeanPostProcessor> beanPostProcessorsMap =
                beanFactory.getBeansOfType(ServiceAnnotationBeanPostProcessor.class);

        Assert.assertEquals(4, beanPostProcessorsMap.size());

        Assert.assertTrue(beanPostProcessorsMap.containsKey("doubleServiceAnnotationBeanPostProcessor"));
        Assert.assertTrue(beanPostProcessorsMap.containsKey("emptyServiceAnnotationBeanPostProcessor"));
        Assert.assertTrue(beanPostProcessorsMap.containsKey("serviceAnnotationBeanPostProcessor"));
        Assert.assertTrue(beanPostProcessorsMap.containsKey("serviceAnnotationBeanPostProcessor2"));

    }

    @ImportResource("META-INF/spring/dubbo-annotation-provider.xml")
    @PropertySource("META-INF/default.properties")
    @ComponentScan("com.alibaba.dubbo.config.spring.context.annotation.provider")
    public static class TestConfiguration {

        @Bean
        public ServiceAnnotationBeanPostProcessor serviceAnnotationBeanPostProcessor
                (@Value("${packagesToScan}") String... packagesToScan) {
            return new ServiceAnnotationBeanPostProcessor(packagesToScan);
        }

        @Bean
        public ServiceAnnotationBeanPostProcessor serviceAnnotationBeanPostProcessor2
                (@Value("${packagesToScan}") String... packagesToScan) {
            return new ServiceAnnotationBeanPostProcessor(packagesToScan);
        }


    }

}
