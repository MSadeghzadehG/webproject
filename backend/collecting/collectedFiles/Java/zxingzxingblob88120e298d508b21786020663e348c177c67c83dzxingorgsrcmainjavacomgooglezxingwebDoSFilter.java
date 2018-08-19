/*
 * Copyright 2015 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.zxing.web;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.annotation.WebFilter;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;

/**
 * A simplistic {@link Filter} that rejects requests from hosts that are sending too many
 * requests in too short a time.
 *
 * @author Sean Owen
 */
@WebFilter({"/w/decode", "/w/chart"})
public final class DoSFilter implements Filter {

  private Timer timer;
  private DoSTracker sourceAddrTracker;

  @Override
  public void init(FilterConfig filterConfig) {
    timer = new Timer("DoSFilter");
    sourceAddrTracker = new DoSTracker(timer, 500, TimeUnit.MILLISECONDS.convert(5, TimeUnit.MINUTES), 10_000);
    timer.scheduleAtFixedRate(
        new TimerTask() {
          @Override
          public void run() {
            System.gc();
          }
        }, 0L, TimeUnit.MILLISECONDS.convert(15, TimeUnit.MINUTES));
  }

  @Override
  public void doFilter(ServletRequest request,
                       ServletResponse response,
                       FilterChain chain) throws IOException, ServletException {
    if (isBanned((HttpServletRequest) request)) {
      HttpServletResponse servletResponse = (HttpServletResponse) response;
      servletResponse.sendError(HttpServletResponse.SC_FORBIDDEN);
    } else {
      chain.doFilter(request, response);
    }
  }

  private boolean isBanned(HttpServletRequest request) {
    String remoteIPAddress = request.getHeader("x-forwarded-for");
    if (remoteIPAddress == null) {
      remoteIPAddress = request.getRemoteAddr();
    }
    return sourceAddrTracker.isBanned(remoteIPAddress);
  }

  @Override
  public void destroy() {
    if (timer != null) {
      timer.cancel();
    }
  }

}