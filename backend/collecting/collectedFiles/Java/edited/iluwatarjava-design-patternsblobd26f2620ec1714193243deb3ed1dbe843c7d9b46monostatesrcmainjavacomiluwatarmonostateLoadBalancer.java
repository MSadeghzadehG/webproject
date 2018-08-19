
package com.iluwatar.monostate;

import java.util.ArrayList;
import java.util.List;



public class LoadBalancer {
  private static List<Server> servers = new ArrayList<>();
  private static int lastServedId;

  static {
    int id = 0;
    servers.add(new Server("localhost", 8081, ++id));
    servers.add(new Server("localhost", 8080, ++id));
    servers.add(new Server("localhost", 8082, ++id));
    servers.add(new Server("localhost", 8083, ++id));
    servers.add(new Server("localhost", 8084, ++id));
  }

  
  public final void addServer(Server server) {
    synchronized (servers) {
      servers.add(server);
    }

  }

  public final int getNoOfServers() {
    return servers.size();
  }

  public static int getLastServedId() {
    return lastServedId;
  }

  
  public synchronized void serverRequest(Request request) {
    if (lastServedId >= servers.size()) {
      lastServedId = 0;
    }
    Server server = servers.get(lastServedId++);
    server.serve(request);
  }
  
}
