
package com.alibaba.dubbo.config.url.test;


import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

public class ExporterSideConfigUrlTest extends UrlTestBase {

    private static final Logger log = LoggerFactory.getLogger(ExporterSideConfigUrlTest.class);

                @BeforeClass
    public static void start() {


    }


    @Before
    public void setUp() {

        initServConf();

        return;
    }

    @After()
    public void teardown() {
    }

    @Test
    public void exporterMethodConfigUrlTest() {

        verifyExporterUrlGeneration(methodConfForService, methodConfForServiceTable);
    }

    @Test
    public void exporterServiceConfigUrlTest() {

        verifyExporterUrlGeneration(servConf, servConfTable);
    }

    @Test
    public void exporterProviderConfigUrlTest() {

        verifyExporterUrlGeneration(provConf, provConfTable);
    }

    @Test
    public void exporterRegistryConfigUrlTest() {

            }


    protected <T> void verifyExporterUrlGeneration(T config, Object[][] dataTable) {

                        fillConfigs(config, dataTable, TESTVALUE1);

                        servConf.export();
        String paramStringFromDb = getProviderParamString();
        try {
            paramStringFromDb = URLDecoder.decode(paramStringFromDb, "UTF-8");
        } catch (UnsupportedEncodingException e) {
                    }


        assertUrlStringWithLocalTable(paramStringFromDb, dataTable, config.getClass().getName(), TESTVALUE1);


                        servConf.unexport();
    }
}