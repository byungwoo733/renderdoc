package @RENDERDOC_ANDROID_PACKAGE_NAME@;
import android.app.Activity;
import android.view.WindowManager;

public class Loader extends android.app.NativeActivity
{
    /* load our native library */
    static {
        System.loadLibrary("renderdoccmd"); // this will load VkLayer_GLES_RenderDoc as well
    }

    @Override
    protected void onCreate(android.os.Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Popup a dialog if we haven't granted Android storage permissions.
        requestPermissions(new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        // Request is asynchronous, so prevent connection to server until permissions granted.
        while(checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
            != android.content.pm.PackageManager.PERMISSION_GRANTED)
        {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}
