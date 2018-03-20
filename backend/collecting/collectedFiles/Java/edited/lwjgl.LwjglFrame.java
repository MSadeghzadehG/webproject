

package com.badlogic.gdx.backends.lwjgl;

import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Point;

import javax.swing.JFrame;

import com.badlogic.gdx.ApplicationListener;


public class LwjglFrame extends JFrame {
	LwjglCanvas lwjglCanvas;
	private Thread shutdownHook;

	public LwjglFrame (ApplicationListener listener, String title, int width, int height) {
		super(title);
		LwjglApplicationConfiguration config = new LwjglApplicationConfiguration();
		config.title = title;
		config.width = width;
		config.height = height;
		construct(listener, config);
	}

	public LwjglFrame (ApplicationListener listener, LwjglApplicationConfiguration config) {
		super(config.title);
		construct(listener, config);
	}

	private void construct (ApplicationListener listener, LwjglApplicationConfiguration config) {
		lwjglCanvas = new LwjglCanvas(listener, config) {
			protected void stopped () {
				LwjglFrame.this.dispose();
			}

			protected void setTitle (String title) {
				LwjglFrame.this.setTitle(title);
			}

			protected void setDisplayMode (int width, int height) {
				LwjglFrame.this.getContentPane().setPreferredSize(new Dimension(width, height));
				LwjglFrame.this.getContentPane().invalidate();
				LwjglFrame.this.pack();
				LwjglFrame.this.setLocationRelativeTo(null);
				updateSize(width, height);
			}

			protected void resize (int width, int height) {
				updateSize(width, height);
			}

			protected void start () {
				LwjglFrame.this.start();
			}

			protected void exception (Throwable t) {
				LwjglFrame.this.exception(t);
			}

			protected int getFrameRate () {
				int frameRate = LwjglFrame.this.getFrameRate();
				return frameRate == 0 ? super.getFrameRate() : frameRate;
			}
		};

		setHaltOnShutdown(true);

		setDefaultCloseOperation(EXIT_ON_CLOSE);
		getContentPane().setPreferredSize(new Dimension(config.width, config.height));

		initialize();
		pack();
		Point location = getLocation();
		if (location.x == 0 && location.y == 0) setLocationRelativeTo(null);
		lwjglCanvas.getCanvas().setSize(getSize());

				EventQueue.invokeLater(new Runnable() {
			public void run () {
				addCanvas();
				setVisible(true);
				lwjglCanvas.getCanvas().requestFocus();
			}
		});
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

	protected int getFrameRate () {
		return 0;
	}

	protected void exception (Throwable ex) {
		ex.printStackTrace();
		lwjglCanvas.stop();
	}

	
	protected void initialize () {
	}

	
	protected void addCanvas () {
		getContentPane().add(lwjglCanvas.getCanvas());
	}

	
	protected void start () {
	}

	
	public void updateSize (int width, int height) {
	}

	public LwjglCanvas getLwjglCanvas () {
		return lwjglCanvas;
	}
}
