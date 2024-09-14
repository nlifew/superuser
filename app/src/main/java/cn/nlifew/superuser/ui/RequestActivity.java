package cn.nlifew.superuser.ui;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

public class RequestActivity extends AppCompatActivity implements
        DialogInterface.OnClickListener {
    private static final String TAG = "RequestActivity";

    private String mSocketPath;
    private AlertDialog mDialog;

    private final Handler mH = new Handler();
    private final StringBuilder mMessage = new StringBuilder(32);

    private int mTime = 9000;

    private final Runnable mTimeTask = new Runnable() {
        @Override
        public void run() {
            if ((mTime -= 1000) == 0) {
                onClick(mDialog, DialogInterface.BUTTON_NEGATIVE);
                return;
            }

            final int len = mMessage.length();
            mMessage.delete(len-2, len);
            mMessage.append(mTime/1000).append("秒");

            mDialog.setMessage(mMessage);
            mH.postDelayed(this, 1000);
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Intent intent = getIntent();
        final int caller_uid = intent.getIntExtra("caller_uid", -1);
        mSocketPath = intent.getStringExtra("socket_path");

        Log.d(TAG, "onCreate: uid: " + caller_uid + " socket: " + mSocketPath);

        
        final CharSequence label;
        final Drawable icon;

        try {
            PackageManager pm = getPackageManager();
            String[] pkgs = pm.getPackagesForUid(caller_uid);
            ApplicationInfo info = pm.getApplicationInfo(pkgs[0], 0);
            
            label = info.loadLabel(pm);
            icon = info.loadIcon(pm);
        } catch (Exception e) {
            Log.e(TAG, "onCreate: failed to load application info.", e);
            finish();
            return;
        }

        mMessage.append(label).append("(").append(caller_uid).append(")")
                .append("请求获取超级权限\n\n")
                .append(mTime/1000).append("秒");

        mDialog = new AlertDialog.Builder(this)
                .setIcon(icon)
                .setTitle("SuperUser")
                .setMessage(mMessage)
                .setPositiveButton("授权", this)
                .setNegativeButton("拒绝", this)
                .create();
        mDialog.show();
        mH.postDelayed(mTimeTask, 1000);
    }

    @Override
    protected void onDestroy() {
        mH.removeCallbacks(mTimeTask);
        super.onDestroy();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        replySocketResult(mSocketPath,
                which == DialogInterface.BUTTON_POSITIVE ?
                0 : -1);

        dialog.dismiss();
        finish();
    }

    static {
        System.loadLibrary("request");
    }

    private static native void replySocketResult(String path, int result);
}
