
package com.alibaba.dubbo.examples.rest.api.extension;

import javax.ws.rs.client.ClientRequestContext;
import javax.ws.rs.client.ClientRequestFilter;
import javax.ws.rs.client.ClientResponseContext;
import javax.ws.rs.client.ClientResponseFilter;
import java.io.IOException;


public class ClientTraceFilter implements ClientRequestFilter, ClientResponseFilter {

    public void filter(ClientRequestContext requestContext) throws IOException {
        System.out.println("Client request filter invoked");
    }

    public void filter(ClientRequestContext clientRequestContext, ClientResponseContext clientResponseContext) throws IOException {
        System.out.println("Client response filter invoked");
    }
}
