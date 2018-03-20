
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.LogUtil;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.support.DemoService;
import com.alibaba.dubbo.rpc.support.MockInvocation;
import com.alibaba.dubbo.rpc.support.MyInvoker;

import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class DeprecatedFilterTest {

    Filter deprecatedFilter = new DeprecatedFilter();

    @Test
    public void testDeprecatedFilter() {
        URL url = URL.valueOf("test:        LogUtil.start();
        deprecatedFilter.invoke(new MyInvoker<DemoService>(url), new MockInvocation());
        assertEquals(1,
                LogUtil.findMessage("The service method com.alibaba.dubbo.rpc.support.DemoService.echo(String) is DEPRECATED"));
        LogUtil.stop();
    }
}