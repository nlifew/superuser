package cn.nlifew.superuser.broadcast;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;
import android.widget.Toast;

public class ResultReceiver extends BroadcastReceiver {
    private static final String TAG = "ResultReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        final int caller_uid = intent.getIntExtra("caller_uid", -1);
        final int result = intent.getIntExtra("su_result", -1);

        Log.d(TAG, "onReceive: caller_uid " + caller_uid + " result " + result);

        final CharSequence caller;

        try {
            PackageManager pm = context.getPackageManager();
            String[] pkgs = pm.getPackagesForUid(caller_uid);
            ApplicationInfo info = pm.getApplicationInfo(pkgs[0], 0);
            caller = info.loadLabel(pm);
        } catch (Exception e) {
            Log.e(TAG, "onReceive: ", e);
            return;
        }

        String msg = "已" + (result == 0 ? "授予" : "拒绝") + caller + "获取超级用户权限";
        Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
    }
}
