

package com.google.zxing.web;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.annotation.WebFilter;
import java.io.IOException;


@WebFilter({"/", "/index.jspx", "/w/", "/w/index.jspx"})
public final class WelcomeFilter extends AbstractFilter {

  @Override
  public void doFilter(ServletRequest servletRequest,
                       ServletResponse servletResponse,
                       FilterChain filterChain) throws IOException, ServletException {
    redirect(servletResponse, "/w/decode.jspx");
  }

}
