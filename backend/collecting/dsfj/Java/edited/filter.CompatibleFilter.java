
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.CompatibleTypeUtils;
import com.alibaba.dubbo.common.utils.PojoUtils;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.RpcResult;

import java.lang.reflect.Method;
import java.lang.reflect.Type;


public class CompatibleFilter implements Filter {

    private static Logger logger = LoggerFactory.getLogger(CompatibleFilter.class);

    public Result invoke(Invoker<?> invoker, Invocation invocation) throws RpcException {
        Result result = invoker.invoke(invocation);
        if (!invocation.getMethodName().startsWith("$") && !result.hasException()) {
            Object value = result.getValue();
            if (value != null) {
                try {
                    Method method = invoker.getInterface().getMethod(invocation.getMethodName(), invocation.getParameterTypes());
                    Class<?> type = method.getReturnType();
                    Object newValue;
                    String serialization = invoker.getUrl().getParameter(Constants.SERIALIZATION_KEY);
                    if ("json".equals(serialization)
                            || "fastjson".equals(serialization)) {
                        Type gtype = method.getGenericReturnType();
                        newValue = PojoUtils.realize(value, type, gtype);
                    } else if (!type.isInstance(value)) {
                        newValue = PojoUtils.isPojo(type)
                                ? PojoUtils.realize(value, type)
                                : CompatibleTypeUtils.compatibleTypeConvert(value, type);

                    } else {
                        newValue = value;
                    }
                    if (newValue != value) {
                        result = new RpcResult(newValue);
                    }
                } catch (Throwable t) {
                    logger.warn(t.getMessage(), t);
                }
            }
        }
        return result;
    }

}