
package com.alibaba.dubbo.rpc.protocol.rest;

import com.alibaba.dubbo.common.utils.StringUtils;
import com.alibaba.dubbo.rpc.RpcContext;

import org.jboss.resteasy.spi.ResteasyProviderFactory;

import javax.annotation.Priority;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.ws.rs.client.ClientRequestContext;
import javax.ws.rs.client.ClientRequestFilter;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerRequestFilter;
import java.io.IOException;
import java.util.Map;

@Priority(Integer.MIN_VALUE + 1)
public class RpcContextFilter implements ContainerRequestFilter, ClientRequestFilter {

    private static final String DUBBO_ATTACHMENT_HEADER = "Dubbo-Attachments";

        private static final int MAX_HEADER_SIZE = 8 * 1024;

    public void filter(ContainerRequestContext requestContext) throws IOException {
        HttpServletRequest request = ResteasyProviderFactory.getContextData(HttpServletRequest.class);
        RpcContext.getContext().setRequest(request);

                if (request != null && RpcContext.getContext().getRemoteAddress() == null) {
            RpcContext.getContext().setRemoteAddress(request.getRemoteAddr(), request.getRemotePort());
        }

        RpcContext.getContext().setResponse(ResteasyProviderFactory.getContextData(HttpServletResponse.class));

        String headers = requestContext.getHeaderString(DUBBO_ATTACHMENT_HEADER);
        if (headers != null) {
            for (String header : headers.split(",")) {
                int index = header.indexOf("=");
                if (index > 0) {
                    String key = header.substring(0, index);
                    String value = header.substring(index + 1);
                    if (!StringUtils.isEmpty(key)) {
                        RpcContext.getContext().setAttachment(key.trim(), value.trim());
                    }
                }
            }
        }
    }

    public void filter(ClientRequestContext requestContext) throws IOException {
        int size = 0;
        for (Map.Entry<String, String> entry : RpcContext.getContext().getAttachments().entrySet()) {
            if (entry.getValue().contains(",") || entry.getValue().contains("=")
                    || entry.getKey().contains(",") || entry.getKey().contains("=")) {
                throw new IllegalArgumentException("The attachments of " + RpcContext.class.getSimpleName() + " must not contain ',' or '=' when using rest protocol");
            }

                        size += entry.getValue().getBytes("UTF-8").length;
            if (size > MAX_HEADER_SIZE) {
                throw new IllegalArgumentException("The attachments of " + RpcContext.class.getSimpleName() + " is too big");
            }

            StringBuilder attachments = new StringBuilder();
            attachments.append(entry.getKey());
            attachments.append("=");
            attachments.append(entry.getValue());
            requestContext.getHeaders().add(DUBBO_ATTACHMENT_HEADER, attachments.toString());
        }
    }
}
