

package com.google.zxing.web;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.annotation.WebFilter;
import javax.servlet.http.HttpServletRequest;
import java.io.IOException;


@WebFilter("/w/decode.jspx")
public final class HTTPSFilter extends AbstractFilter {

  @Override
  public void doFilter(ServletRequest servletRequest, 
                       ServletResponse servletResponse, 
                       FilterChain chain) throws IOException, ServletException {
    if (servletRequest.isSecure()) {
      chain.doFilter(servletRequest, servletResponse);
    } else {
      HttpServletRequest request = (HttpServletRequest) servletRequest; 
      String target = request.getRequestURL().toString().replaceFirst("http:      redirect(servletResponse, target);
    }
  }

}
