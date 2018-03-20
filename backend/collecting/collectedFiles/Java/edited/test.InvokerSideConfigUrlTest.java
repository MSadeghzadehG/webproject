
package com.alibaba.dubbo.config.url.test;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.config.ApplicationConfig;
import com.alibaba.dubbo.config.ConsumerConfig;
import com.alibaba.dubbo.config.MethodConfig;
import com.alibaba.dubbo.config.ReferenceConfig;
import com.alibaba.dubbo.config.RegistryConfig;
import com.alibaba.dubbo.config.api.DemoService;
import com.alibaba.dubbo.config.support.MockRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.util.Arrays;


public class InvokerSideConfigUrlTest extends UrlTestBase {
    private static final Logger log = LoggerFactory.getLogger(InvokerSideConfigUrlTest.class);

                private ApplicationConfig appConfForConsumer;
    private ApplicationConfig appConfForReference;
    private RegistryConfig regConfForConsumer;
    private RegistryConfig regConfForReference;
    private MethodConfig methodConfForReference;
    private ConsumerConfig consumerConf;
    private ReferenceConfig<DemoService> refConf;

    private Object appConfForConsumerTable[][] = {
            {"", "", "", "", "", "", "", "", "", ""},
    };

    private Object appConfForReferenceTable[][] = {
            {"", "", "", "", "", "", "", "", "", ""},
    };

    private Object regConfForConsumerTable[][] = {
            {"subscribe", "subscribe", "boolean", true, false, "", "", "", "", ""},
            {"dynamic", "dynamic", "boolean", true, false, "", "", "", "", ""},
    };

    private Object regConfForReferenceTable[][] = {
            {"timeout", "registry.timeout", "int", 5000, 9000, "", "", "", "", ""},
            {"file", "registry.file", "string", "", "regConfForServiceTable.log", "", "", "", "", ""},
            {"wait", "registry.wait", "int", 0, 9000, "", "", "", "", ""},
            {"transport", "registry.transporter", "string", "netty", "mina", "", "", "", "", ""},
            {"subscribe", "subscribe", "boolean", true, false, "", "", "", "", ""},
            {"dynamic", "dynamic", "boolean", true, false, "", "", "", "", ""},
    };

    private Object methodConfForReferenceTable[][] = {
            {"actives", "eatTiger.actives", "int", 0, 90, "", "", "", "", ""},
            {"executes", "eatTiger.executes", "int", 0, 90, "", "", "", "", ""},
            {"deprecated", "eatTiger.deprecated", "boolean", false, true, "", "", "", "", ""},
            {"async", "eatTiger.async", "boolean", false, true, "", "", "", "", ""},
            {"timeout", "eatTiger.timeout", "int", 0, 90, "", "", "", "", ""},
    };

    private Object refConfTable[][] = {

            {"timeout", "timeout", "int", 5000, 3000, "", "", "", "", ""},
            {"retries", "retries", "int", 2, 5, "", "", "", "", ""},
            {"connections", "connections", "boolean", 100, 20, "", "", "", "", ""},
            {"loadbalance", "loadbalance", "string", "random", "roundrobin", "leastactive", "", "", ""},
            {"async", "async", "boolean", false, true, "", "", "", "", ""},
                        {"check", "check", "boolean", false, true, "", "", "", "", ""},
                                                {"mock", "mock", "string", "false", "false", "", "", "", "", ""},
            {"proxy", "proxy", "boolean", "javassist", "jdk", "", "", "", "", ""},
            {"client", "client", "string", "netty", "mina", "", "", "", "", ""},
            {"client", "client", "string", "netty", "mina", "", "", "", "", ""},
            {"owner", "owner", "string", "", "haomin,ludvik", "", "", "", "", ""},
            {"actives", "actives", "int", 0, 30, "", "", "", "", ""},
            {"cluster", "cluster", "string", "failover", "failfast", "failsafe", "failback", "forking", "", ""},
                                        };

    private Object consumerConfTable[][] = {
            {"timeout", "default.timeout", "int", 5000, 8000, "", "", "", "", ""},
            {"retries", "default.retries", "int", 2, 5, "", "", "", "", ""},
            {"loadbalance", "default.loadbalance", "string", "random", "leastactive", "", "", "", "", ""},
            {"async", "default.async", "boolean", false, true, "", "", "", "", ""},
            {"connections", "default.connections", "int", 100, 5, "", "", "", "", ""},
            {"check", "check", "boolean", true, false, "", "", "", "", ""},
            {"proxy", "proxy", "string", "javassist", "jdk", "javassist", "", "", "", ""},
            {"owner", "owner", "string", "", "haomin", "", "", "", "", ""},
            {"actives", "default.actives", "int", 0, 5, "", "", "", "", ""},
            {"cluster", "default.cluster", "string", "failover", "forking", "", "", "", "", ""},
            {"filter", "", "string", "", "", "", "", "", "", ""},
            {"listener", "", "string", "", "", "", "", "", "", ""},
    };

            
    @BeforeClass
    public static void start() {

            }


    @Before
    public void setUp() {

        initServConf();
        initRefConf();

        return;
    }

    @After()
    public void teardown() {

            }


    @Test
    public void consumerConfUrlTest() {
        verifyInvokerUrlGeneration(consumerConf, consumerConfTable);
    }

    @Test
    public void refConfUrlTest() {
        verifyInvokerUrlGeneration(refConf, refConfTable);
    }

    @Ignore("parameter on register center will not be merged any longer with query parameter request from the consumer")
    @Test
    public void regConfForConsumerUrlTest() {

        verifyInvokerUrlGeneration(regConfForConsumer, regConfForConsumerTable);

    }

                private void initRefConf() {

        appConfForConsumer = new ApplicationConfig();
        appConfForReference = new ApplicationConfig();
        regConfForConsumer = new RegistryConfig();
        regConfForReference = new RegistryConfig();
        methodConfForReference = new MethodConfig();

        refConf = new ReferenceConfig<DemoService>();
        consumerConf = new ConsumerConfig();

        methodConfForReference.setName("sayName");
        regConfForReference.setAddress("127.0.0.1:9090");
        regConfForReference.setProtocol("mockregistry");
        appConfForReference.setName("ConfigTests");
        refConf.setInterface("com.alibaba.dubbo.config.api.DemoService");

        refConf.setApplication(appConfForReference);
        consumerConf.setApplication(appConfForConsumer);

        refConf.setRegistry(regConfForReference);
        consumerConf.setRegistry(regConfForConsumer);

        refConf.setConsumer(consumerConf);

        refConf.setMethods(Arrays.asList(new MethodConfig[]{methodConfForReference}));

        refConf.setScope(Constants.SCOPE_REMOTE);
    }

    private <T> void verifyInvokerUrlGeneration(T config, Object[][] dataTable) {
        servConf.export();

        fillConfigs(config, dataTable, TESTVALUE1);
        refConf.get();

        String subScribedUrlStr = getSubscribedUrlString();

        System.out.println("url string=========:" + subScribedUrlStr);
        String configName = config.getClass().getName();
        int column = TESTVALUE1;

        assertUrlStringWithLocalTable(subScribedUrlStr, dataTable, configName, column);

        try {
            refConf.destroy();
        } catch (Exception e) {
        }
    }

    private String getSubscribedUrlString() {
        return MockRegistry.getSubscribedUrl().toString();
    }
}