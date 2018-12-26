
package com.alibaba.dubbo.rpc.cluster.router.file;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.IOUtils;
import com.alibaba.dubbo.rpc.cluster.Router;
import com.alibaba.dubbo.rpc.cluster.RouterFactory;
import com.alibaba.dubbo.rpc.cluster.router.script.ScriptRouterFactory;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;

public class FileRouterFactory implements RouterFactory {

    public static final String NAME = "file";

    private RouterFactory routerFactory;

    public void setRouterFactory(RouterFactory routerFactory) {
        this.routerFactory = routerFactory;
    }

    public Router getRouter(URL url) {
        try {
                                    String protocol = url.getParameter(Constants.ROUTER_KEY, ScriptRouterFactory.NAME);             String type = null;             String path = url.getPath();
            if (path != null) {
                int i = path.lastIndexOf('.');
                if (i > 0) {
                    type = path.substring(i + 1);
                }
            }
            String rule = IOUtils.read(new FileReader(new File(url.getAbsolutePath())));

            boolean runtime = url.getParameter(Constants.RUNTIME_KEY, false);
            URL script = url.setProtocol(protocol).addParameter(Constants.TYPE_KEY, type).addParameter(Constants.RUNTIME_KEY, runtime).addParameterAndEncoded(Constants.RULE_KEY, rule);

            return routerFactory.getRouter(script);
        } catch (IOException e) {
            throw new IllegalStateException(e.getMessage(), e);
        }
    }

}