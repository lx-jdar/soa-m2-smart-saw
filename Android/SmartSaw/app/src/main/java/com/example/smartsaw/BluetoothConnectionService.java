package com.example.smartsaw;

import android.content.Context;

import androidx.appcompat.app.AppCompatActivity;

public interface BluetoothConnectionService {
    public static final String ACTION_DATA_RECEIVE ="smartsaw.intent.DATA_RECEIVE";

    void setActivity(AppCompatActivity activity);

    void setContext(Context context);

    void onCreateBluetooth();

    void onResumeBluetooth();

}
