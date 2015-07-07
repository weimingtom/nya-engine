package nya;

import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

public class native_activity extends Activity implements SurfaceHolder.Callback
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);
        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(this);
    }

    @Override
    protected void onStart()
    {
        super.onStart();
        native_spawn_main();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        native_resume();
    }
    
    @Override
    protected void onPause()
    {
        super.onPause();
        native_pause();
    }

    @Override
    protected void onStop()
    {
        super.onStop();
        native_exit();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        native_set_surface(holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
    }

    public void surfaceDestroyed(SurfaceHolder holder)
    {
        native_set_surface(null);
    }

    public static native void native_spawn_main();
    public static native void native_resume();
    public static native void native_pause();
    public static native void native_exit();
    public static native void native_set_surface(Surface surface);

    static
    {
        System.loadLibrary("nya_native");
    }
}
