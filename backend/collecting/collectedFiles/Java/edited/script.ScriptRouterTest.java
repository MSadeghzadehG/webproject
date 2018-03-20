
package com.alibaba.dubbo.rpc.cluster.router.script;


import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.RpcInvocation;
import com.alibaba.dubbo.rpc.cluster.Router;
import com.alibaba.dubbo.rpc.cluster.router.MockInvoker;

import junit.framework.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

public class ScriptRouterTest {

    private URL SCRIPT_URL = URL.valueOf("script:
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    @Before
    public void setUp() throws Exception {
    }

    private URL getRouteUrl(String rule) {
        return SCRIPT_URL.addParameterAndEncoded(Constants.RULE_KEY, rule);
    }

    @Test
    public void testRoute_ReturnAll() {
        Router router = new ScriptRouterFactory().getRouter(getRouteUrl("function route(op1,op2){return op1} route(invokers)"));
        List<Invoker<String>> invokers = new ArrayList<Invoker<String>>();
        invokers.add(new MockInvoker<String>());
        invokers.add(new MockInvoker<String>());
        invokers.add(new MockInvoker<String>());
        List<Invoker<String>> fileredInvokers = router.route(invokers, invokers.get(0).getUrl(), new RpcInvocation());
        Assert.assertEquals(invokers, fileredInvokers);
    }

    @Test
    public void testRoute_PickInvokers() {
        String rule = "var result = new java.util.ArrayList(invokers.size());" +
                "for (i=0;i<invokers.size(); i++){ " +
                "if (invokers.get(i).isAvailable()) {" +
                "result.add(invokers.get(i)) ;" +
                "}" +
                "} ; " +
                "return result;";
        String script = "function route(invokers,invocation,context){" + rule + "} route(invokers,invocation,context)";
        Router router = new ScriptRouterFactory().getRouter(getRouteUrl(script));

        List<Invoker<String>> invokers = new ArrayList<Invoker<String>>();
        Invoker<String> invoker1 = new MockInvoker<String>(false);
        Invoker<String> invoker2 = new MockInvoker<String>(true);
        Invoker<String> invoker3 = new MockInvoker<String>(true);
        invokers.add(invoker1);
        invokers.add(invoker2);
        invokers.add(invoker3);
        List<Invoker<String>> fileredInvokers = router.route(invokers, invokers.get(0).getUrl(), new RpcInvocation());
        Assert.assertEquals(2, fileredInvokers.size());
        Assert.assertEquals(invoker2, fileredInvokers.get(0));
        Assert.assertEquals(invoker3, fileredInvokers.get(1));
    }
    }