

package com.badlogic.gdx.tests.net;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Net.Protocol;
import com.badlogic.gdx.net.ServerSocket;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.tests.utils.GdxTest;


public class PingPongSocketExample extends GdxTest {
	@Override
	public void create () {
						new Thread(new Runnable() {
			@Override
			public void run () {
				ServerSocketHints hints = new ServerSocketHints();
				ServerSocket server = Gdx.net.newServerSocket(Protocol.TCP, "localhost", 9999, hints);
								Socket client = server.accept(null);
								try {
					String message = new BufferedReader(new InputStreamReader(client.getInputStream())).readLine();
					Gdx.app.log("PingPongSocketExample", "got client message: " + message);
					client.getOutputStream().write("PONG\n".getBytes());
				} catch (IOException e) {
					Gdx.app.log("PingPongSocketExample", "an error occured", e);
				}
			}
		}).start();

						SocketHints hints = new SocketHints();
		Socket client = Gdx.net.newClientSocket(Protocol.TCP, "localhost", 9999, hints);
		try {
			client.getOutputStream().write("PING\n".getBytes());
			String response = new BufferedReader(new InputStreamReader(client.getInputStream())).readLine();
			Gdx.app.log("PingPongSocketExample", "got server message: " + response);
		} catch (IOException e) {
			Gdx.app.log("PingPongSocketExample", "an error occured", e);
		}
	}
}
