

package com.badlogic.gdx.backends.lwjgl;

import com.badlogic.gdx.ApplicationListener;

import java.awt.Dimension;

import javax.swing.JFrame;


public class LwjglAWTFrame extends JFrame {
	final LwjglAWTCanvas lwjglAWTCanvas;
	private Thread shutdownHook;

	public LwjglAWTFrame (ApplicationListener listener, String title, int width, int height) {
		super(title);

		lwjglAWTCanvas = new LwjglAWTCanvas(listener) {
			protected void stopped () {
				LwjglAWTFrame.this.dispose();
			}

			protected void setTitle (String title) {
				LwjglAWTFrame.this.setTitle(title);
			}

			protected void setDisplayMode (int width, int height) {
				LwjglAWTFrame.this.getContentPane().setPreferredSize(new Dimension(width, height));
				LwjglAWTFrame.this.getContentPane().invalidate();
				LwjglAWTFrame.this.pack();
				LwjglAWTFrame.this.setLocationRelativeTo(null);
				updateSize(width, height);
			}

			protected void resize (int width, int height) {
				updateSize(width, height);
			}

			protected void start () {
				LwjglAWTFrame.this.start();
			}
		};
		getContentPane().add(lwjglAWTCanvas.getCanvas());

		setHaltOnShutdown(true);

		setDefaultCloseOperation(EXIT_ON_CLOSE);
		getContentPane().setPreferredSize(new Dimension(width, height));
		initialize();
		pack();
		setLocationRelativeTo(null);
		setVisible(true);
		lwjglAWTCanvas.getCanvas().requestFocus();
	}

	
	public void setHaltOnShutdown (boolean halt) {
		if (halt) {
			if (shutdownHook != null) return;
			shutdownHook = new Thread() {
				public void run () {
					Runtime.getRuntime().halt(0); 				}
			};
			Runtime.getRuntime().addShutdownHook(shutdownHook);
		} else if (shutdownHook != null) {
			Runtime.getRuntime().removeShutdownHook(shutdownHook);
			shutdownHook = null;
		}
	}

	
	protected void initialize () {
	}

	
	protected void start () {
	}

	
	public void updateSize (int width, int height) {
	}

	public LwjglAWTCanvas getLwjglAWTCanvas () {
		return lwjglAWTCanvas;
	}
}
