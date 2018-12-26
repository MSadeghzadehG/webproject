

package com.badlogic.gdx.backends.android;

import java.lang.reflect.Method;

import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.EGLConfigChooser;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.backends.android.surfaceview.GLSurfaceView20;
import com.badlogic.gdx.backends.android.surfaceview.GLSurfaceView20API18;
import com.badlogic.gdx.backends.android.surfaceview.GLSurfaceViewAPI18;
import com.badlogic.gdx.backends.android.surfaceview.ResolutionStrategy;
import com.badlogic.gdx.utils.GdxRuntimeException;


public final class AndroidGraphicsLiveWallpaper extends AndroidGraphics {

	public AndroidGraphicsLiveWallpaper (AndroidLiveWallpaper lwp, AndroidApplicationConfiguration config,
		ResolutionStrategy resolutionStrategy) {
		super(lwp, config, resolutionStrategy, false);
	}

			
			SurfaceHolder getSurfaceHolder () {
		synchronized (((AndroidLiveWallpaper)app).service.sync) {
			return ((AndroidLiveWallpaper)app).service.getSurfaceHolder();
		}
	}

	
			@Override
	protected View createGLSurfaceView (AndroidApplicationBase application, final ResolutionStrategy resolutionStrategy) {
		if (!checkGL20()) throw new GdxRuntimeException("Libgdx requires OpenGL ES 2.0");

		EGLConfigChooser configChooser = getEglConfigChooser();
		int sdkVersion = android.os.Build.VERSION.SDK_INT;
		if (sdkVersion <= 10 && config.useGLSurfaceView20API18) {
			GLSurfaceView20API18 view = new GLSurfaceView20API18(application.getContext(), resolutionStrategy) {
				@Override
				public SurfaceHolder getHolder () {
					return getSurfaceHolder();
				}

								public void onDestroy () {
					onDetachedFromWindow(); 				}
			};
			if (configChooser != null)
				view.setEGLConfigChooser(configChooser);
			else
				view.setEGLConfigChooser(config.r, config.g, config.b, config.a, config.depth, config.stencil);
			view.setRenderer(this);
			return view;
		}
		else {
			GLSurfaceView20 view = new GLSurfaceView20(application.getContext(), resolutionStrategy) {
				@Override
				public SurfaceHolder getHolder () {
					return getSurfaceHolder();
				}
	
								public void onDestroy () {
					onDetachedFromWindow(); 				}
			};
	
			if (configChooser != null)
				view.setEGLConfigChooser(configChooser);
			else
				view.setEGLConfigChooser(config.r, config.g, config.b, config.a, config.depth, config.stencil);
			view.setRenderer(this);
			return view;
		}
	}

			public void onDestroyGLSurfaceView () {
		if (view != null) {
			if (view instanceof GLSurfaceView || view instanceof GLSurfaceViewAPI18) {
				try {
										view.getClass().getMethod("onDestroy").invoke(view);
					if (AndroidLiveWallpaperService.DEBUG)
						Log.d(AndroidLiveWallpaperService.TAG,
							" > AndroidLiveWallpaper - onDestroy() stopped GLThread managed by GLSurfaceView");
				} catch (Throwable t) {
															Log.e(AndroidLiveWallpaperService.TAG,
						"failed to destroy GLSurfaceView's thread! GLSurfaceView.onDetachedFromWindow impl changed since API lvl 16!");
					t.printStackTrace();
				}
			}
		}
	}

	@Override
	void resume () {
		synchronized (synch) {
			running = true;
			resume = true;

			while (resume) {
				try {
					requestRendering();
					synch.wait();
				} catch (InterruptedException ignored) {
					Gdx.app.log("AndroidGraphics", "waiting for resume synchronization failed!");
				}
			}
		}
	}

	@Override
	public void onDrawFrame (javax.microedition.khronos.opengles.GL10 gl) {
		long time = System.nanoTime();
		deltaTime = (time - lastFrameTime) / 1000000000.0f;
		lastFrameTime = time;

				if (!resume) {
			mean.addValue(deltaTime);
		} else {
			deltaTime = 0;
		}

		boolean lrunning = false;
		boolean lpause = false;
		boolean ldestroy = false;
		boolean lresume = false;

		synchronized (synch) {
			lrunning = running;
			lpause = pause;
			ldestroy = destroy;
			lresume = resume;

			if (resume) {
				resume = false;
				synch.notifyAll();
			}

			if (pause) {
				pause = false;
				synch.notifyAll();
			}

			if (destroy) {
				destroy = false;
				synch.notifyAll();
			}
		}

		if (lresume) {
						app.getApplicationListener().resume();
			Gdx.app.log("AndroidGraphics", "resumed");
		}

										if (lrunning) {

						synchronized (app.getRunnables()) {
				app.getExecutedRunnables().clear();
				app.getExecutedRunnables().addAll(app.getRunnables());
				app.getRunnables().clear();

				for (int i = 0; i < app.getExecutedRunnables().size; i++) {
					try {
						app.getExecutedRunnables().get(i).run();
					} catch (Throwable t) {
						t.printStackTrace();
					}
				}
			}
			

			app.getInput().processEvents();
			frameId++;
			app.getApplicationListener().render();
		}

				if (lpause) {
			app.getApplicationListener().pause();
						Gdx.app.log("AndroidGraphics", "paused");
		}

				if (ldestroy) {
			app.getApplicationListener().dispose();
						Gdx.app.log("AndroidGraphics", "destroyed");
		}

		if (time - frameStart > 1000000000) {
			fps = frames;
			frames = 0;
			frameStart = time;
		}
		frames++;
	}

	@Override
	protected void logManagedCachesStatus() {
				if (AndroidLiveWallpaperService.DEBUG) {
			super.logManagedCachesStatus();
		}
	}
}
