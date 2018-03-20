
package com.alibaba.dubbo.remoting.http;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;


public interface HttpHandler {

    
    void handle(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException;

}