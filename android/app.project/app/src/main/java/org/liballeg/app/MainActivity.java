package org.liballeg.app;
import org.liballeg.android.AllegroActivity;
import android.content.Intent;
import android.net.Uri;
import android.content.Context;
import android.util.Log;

public class MainActivity extends AllegroActivity
{
    static void loadLibrary(String name)
    {
        try
        {
            // try loading the debug library first.
            Log.d("loadLibrary", name + "-debug");
            System.loadLibrary(name + "-debug");
        }
        catch (UnsatisfiedLinkError e)
        {
            try
            {
                // If it fails load the release library.
                Log.d("loadLibrary", name);
                System.loadLibrary(name);
            }
            catch (UnsatisfiedLinkError e2)
            {
                // We still continue as failing to load an addon may
                // not be a fatal error - for example if the TTF was
                // not built we can still run an example which does not
                // need that addon.
                Log.d("loadLibrary", name + " FAILED");
            }
        }
    }

    static
    {
        loadLibrary("allegro");
        loadLibrary("allegro_primitives");
        loadLibrary("allegro_image");
        loadLibrary("allegro_font");
        loadLibrary("allegro_ttf");
        loadLibrary("allegro_audio");
        loadLibrary("allegro_acodec");
        loadLibrary("allegro_color");
        loadLibrary("allegro_memfile");
        loadLibrary("allegro_physfs");
        loadLibrary("allegro_video");
        loadLibrary("app"); // needed for access to our native code
    }
    native void nativeOnEditComplete(String s);
    public MainActivity()
    {
        super("libapp.so");
    }

    /* for handling data returned from spawned activities
	 * requestCode is value we passed to startActivityForResult,
	 * resultCode is RESULT_OK or RESULT_CANCELED,
	 * data is data returned from activity */
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if(requestCode == 0)
		{
			if(resultCode == RESULT_OK)
			{
				nativeOnEditComplete(data.getStringExtra("EXTRA_TEXT"));
			}
			else
			{
			}
		}
	}

	public void OpenEditBox(String title, String initial, String flags)
	{
		Context mContext = getApplicationContext();
		Intent intent = new Intent(this, EditBoxActivity.class);
		intent.putExtra("EXTRA_TITLE", title);
		intent.putExtra("EXTRA_INITIAL", initial);
		intent.putExtra("EXTRA_FLAGS", flags);
        startActivityForResult(intent, 0);
	}

	public void openURL(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		startActivity(intent);
	}
}
