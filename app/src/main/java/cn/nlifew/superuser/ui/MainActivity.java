package cn.nlifew.superuser.ui;

import android.content.pm.PackageInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;

import cn.nlifew.superuser.R;

public class MainActivity extends AppCompatActivity implements
        View.OnClickListener {

    private static final String TAG = "MainActivity";

    private Handler mWorker;

    private final Runnable mSUTask = new Runnable() {
        @Override
        public void run() {
            try {
                String file = getApplicationInfo().nativeLibraryDir + "/libsu.so";

                ProcessBuilder builder = new ProcessBuilder(file);
                builder.redirectErrorStream(true);

                final Process p = builder.start();
                final Writer writer = new OutputStreamWriter(p.getOutputStream());
                writer.write("id\n");
                writer.flush();
                writer.write("exit\n");
                writer.flush();

                BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));
                String s;
                while ((s = reader.readLine()) != null) {
                    Log.i(TAG, "run: " + s);
                }
                p.waitFor();

                Log.d(TAG, "run: exit code: " + p.exitValue());
            } catch (Exception e) {
                Log.e(TAG, "run: ", e);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        HandlerThread t = new HandlerThread(TAG);
        t.start();
        mWorker = new Handler(t.getLooper());
    }

    @Override
    protected void onDestroy() {
        mWorker.getLooper().quit();
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        mWorker.post(mSUTask);
    }
}
