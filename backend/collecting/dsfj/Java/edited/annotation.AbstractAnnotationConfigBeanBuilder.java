
package com.alibaba.dubbo.config.spring.beans.factory.annotation;

import com.alibaba.dubbo.config.*;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.context.ApplicationContext;
import org.springframework.util.Assert;

import java.lang.annotation.Annotation;
import java.util.List;

import static com.alibaba.dubbo.config.spring.util.BeanFactoryUtils.getBeans;
import static com.alibaba.dubbo.config.spring.util.BeanFactoryUtils.getOptionalBean;


abstract class AbstractAnnotationConfigBeanBuilder<A extends Annotation, B extends AbstractInterfaceConfig> {

    protected final Log logger = LogFactory.getLog(getClass());

    protected final A annotation;

    protected final ApplicationContext applicationContext;

    protected final ClassLoader classLoader;

    protected Object bean;

    protected Class<?> interfaceClass;

    protected AbstractAnnotationConfigBeanBuilder(A annotation, ClassLoader classLoader,
                                                  ApplicationContext applicationContext) {
        Assert.notNull(annotation, "The Annotation must not be null!");
        Assert.notNull(classLoader, "The ClassLoader must not be null!");
        Assert.notNull(applicationContext, "The ApplicationContext must not be null!");
        this.annotation = annotation;
        this.applicationContext = applicationContext;
        this.classLoader = classLoader;

    }

    
    public final B build() throws Exception {

        checkDependencies();

        B bean = doBuild();

        configureBean(bean);

        if (logger.isInfoEnabled()) {
            logger.info(bean + " has been built.");
        }

        return bean;

    }

    private void checkDependencies() {

    }

    
    protected abstract B doBuild();


    protected void configureBean(B bean) throws Exception {

        preConfigureBean(annotation, bean);

        configureRegistryConfigs(bean);

        configureMonitorConfig(bean);

        configureApplicationConfig(bean);

        configureModuleConfig(bean);

        postConfigureBean(annotation, bean);

    }

    protected abstract void preConfigureBean(A annotation, B bean) throws Exception;


    private void configureRegistryConfigs(B bean) {

        String[] registryConfigBeanIds = resolveRegistryConfigBeanNames(annotation);

        List<RegistryConfig> registryConfigs = getBeans(applicationContext, registryConfigBeanIds, RegistryConfig.class);

        bean.setRegistries(registryConfigs);

    }

    private void configureMonitorConfig(B bean) {

        String monitorBeanName = resolveMonitorConfigBeanName(annotation);

        MonitorConfig monitorConfig = getOptionalBean(applicationContext, monitorBeanName, MonitorConfig.class);

        bean.setMonitor(monitorConfig);

    }

    private void configureApplicationConfig(B bean) {

        String applicationConfigBeanName = resolveApplicationConfigBeanName(annotation);

        ApplicationConfig applicationConfig =
                getOptionalBean(applicationContext, applicationConfigBeanName, ApplicationConfig.class);

        bean.setApplication(applicationConfig);

    }

    private void configureModuleConfig(B bean) {

        String moduleConfigBeanName = resolveModuleConfigBeanName(annotation);

        ModuleConfig moduleConfig =
                getOptionalBean(applicationContext, moduleConfigBeanName, ModuleConfig.class);

        bean.setModule(moduleConfig);

    }

    
    protected abstract String resolveModuleConfigBeanName(A annotation);

    
    protected abstract String resolveApplicationConfigBeanName(A annotation);


    
    protected abstract String[] resolveRegistryConfigBeanNames(A annotation);

    
    protected abstract String resolveMonitorConfigBeanName(A annotation);

    
    protected abstract void postConfigureBean(A annotation, B bean) throws Exception;


    public <T extends AbstractAnnotationConfigBeanBuilder<A, B>> T bean(Object bean) {
        this.bean = bean;
        return (T) this;
    }

    public <T extends AbstractAnnotationConfigBeanBuilder<A, B>> T interfaceClass(Class<?> interfaceClass) {
        this.interfaceClass = interfaceClass;
        return (T) this;
    }

}
