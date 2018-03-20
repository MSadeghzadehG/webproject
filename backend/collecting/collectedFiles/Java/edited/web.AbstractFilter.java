

package com.google.zxing.web;

import com.google.common.net.HttpHeaders;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;


abstract class AbstractFilter implements Filter {

  @Override
  public void init(FilterConfig filterConfig) throws ServletException {
      }

  @Override
  public abstract void doFilter(ServletRequest request, 
                       ServletResponse response, 
                       FilterChain chain) throws IOException, ServletException;
  @Override
  public void destroy() {
      }
  
  static void redirect(ServletResponse servletResponse, String location) {
    HttpServletResponse response = (HttpServletResponse) servletResponse;
    response.setStatus(HttpServletResponse.SC_MOVED_PERMANENTLY);
    response.setHeader(HttpHeaders.LOCATION, location);
  }

}
