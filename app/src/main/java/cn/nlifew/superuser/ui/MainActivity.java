package cn.nlifew.superuser.ui;

import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

import cn.nlifew.superuser.R;

public class MainActivity extends AppCompatActivity implements
        View.OnClickListener {

    private static final String TAG = "MainActivity";

    private static final String SU = "/data/local/tmp/su";

    private Handler mWorker;

    private final Runnable mSUTask = new Runnable() {
        @Override
        public void run() {
            try {
                ProcessBuilder builder = new ProcessBuilder(SU);
                builder.redirectErrorStream(true);

                final Process p = builder.start();
                final Writer writer = new OutputStreamWriter(p.getOutputStream());
                writer.write("exit\n");
                writer.flush();

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

        TextView tv = findViewById(R.id.activity_main_info);
        tv.setText("请确保可执行文件的路径在" + SU + "并开启了daemon");

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
