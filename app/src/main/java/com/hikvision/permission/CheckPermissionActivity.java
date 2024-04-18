package com.hikvision.permission;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.TextUtils;
import android.widget.Toast;

import com.hikvision.knxdemo.MainActivity;
import com.hikvision.knxdemo.R;



public class CheckPermissionActivity extends Activity {
    private final int REQUEST_CODE_ASK_PERMISSIONS = 1024;
    private static final String[] REQUEST_PERMISSIONS = new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            //Manifest.permission.RECORD_AUDIO,
            //Manifest.permission.CAMERA
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
//            Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
//            intent.setData(Uri.parse("package:" + this.getPackageName()));
//             //1024为REQUEST_CODE
//            startActivityForResult(intent, 1024);
            requestPermissions(REQUEST_PERMISSIONS, REQUEST_CODE_ASK_PERMISSIONS);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case REQUEST_CODE_ASK_PERMISSIONS:
                if (null == grantResults || grantResults.length < 1) {//monkey maybe enter
                    String toast_text = getResources().getString(R.string.err_permission);
                    Toast.makeText(CheckPermissionActivity.this, toast_text,
                            Toast.LENGTH_SHORT).show();
                    finish();
                    return;
                } else {
                    for (int result : grantResults) {
                        if (result != PackageManager.PERMISSION_GRANTED) {
                            // Permission Denied
                            String toast_text = getResources().getString(R.string.err_permission);
                            Toast.makeText(CheckPermissionActivity.this, toast_text,
                                    Toast.LENGTH_SHORT).show();
                            finish();
                            return;
                        }
                    }
                }
                // Permission Granted
                back2MainActivity();
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }

    }

    private void back2MainActivity() {
        Intent intent = new Intent(this, MainActivity.class);
        if (null != getIntent() && null != getIntent().getAction()) {
            intent.setAction(getIntent().getAction());
        }
        startActivity(intent);
        finish();
    }

    public static boolean jump2PermissionActivity(Activity activity, String action) {
        for (String permission : REQUEST_PERMISSIONS) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if (PackageManager.PERMISSION_GRANTED != activity.checkSelfPermission(permission)) {
                    Intent intent = new Intent(activity, CheckPermissionActivity.class);
                    if (!TextUtils.isEmpty(action)) {
                        intent.setAction(action);
                    }
                    activity.startActivity(intent);
                    return true;
                }
            }
        }
        return false;
    }
}
