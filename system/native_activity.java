package nya;

import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import android.view.View;

public class native_activity extends Activity implements SurfaceHolder.Callback, View.OnTouchListener
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);
        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setOnTouchListener(this);
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
    protected void onDestroy()
    {
        native_exit();
        super.onDestroy();
    }

    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        switch(event.getActionMasked())
        {
            case MotionEvent.ACTION_MOVE:
                for(int i = 0;i<event.getPointerCount();++i)
                    native_touch((int)event.getX(i),(int)event.getY(i),event.getPointerId(i),true);
                break;

            case MotionEvent.ACTION_UP:
                native_touch((int)event.getX(0),(int)event.getY(0),event.getPointerId(0),false);
                break;
            case MotionEvent.ACTION_DOWN:
                native_touch((int)event.getX(0),(int)event.getY(0),event.getPointerId(0),true);
                break;

            case MotionEvent.ACTION_POINTER_UP:
                int i=event.getActionIndex();
                native_touch((int)event.getX(i),(int)event.getY(i),event.getPointerId(i),false);
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                i=event.getActionIndex();
                native_touch((int)event.getX(i),(int)event.getY(i),event.getPointerId(i),true);
                break;

            default:
                break;
        }

        return true;
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        native_set_surface(holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder) {}
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        native_set_surface(null);
    }

    public static native void native_spawn_main();
    public static native void native_resume();
    public static native void native_pause();
    public static native void native_exit();
    public static native void native_touch(int x,int y,int id,boolean pressed);
    public static native void native_set_surface(Surface surface);

    static
    {
        System.loadLibrary("nya_native");
    }
}
