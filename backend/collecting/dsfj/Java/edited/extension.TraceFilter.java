
package com.alibaba.dubbo.examples.rest.api.extension;

import javax.annotation.Priority;
import javax.ws.rs.Priorities;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerRequestFilter;
import javax.ws.rs.container.ContainerResponseContext;
import javax.ws.rs.container.ContainerResponseFilter;
import java.io.IOException;

@Priority(Priorities.USER)
public class TraceFilter implements ContainerRequestFilter, ContainerResponseFilter {

    public void filter(ContainerRequestContext requestContext) throws IOException {
        System.out.println("Request filter invoked: " + requestContext.getUriInfo().getAbsolutePath());
    }

    public void filter(ContainerRequestContext containerRequestContext, ContainerResponseContext containerResponseContext) throws IOException {
        System.out.println("Response filter invoked.");

    }
}