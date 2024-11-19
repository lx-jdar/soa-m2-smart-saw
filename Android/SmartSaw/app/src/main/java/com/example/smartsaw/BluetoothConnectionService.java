package com.example.smartsaw;

import android.content.Context;

import androidx.appcompat.app.AppCompatActivity;

public interface BluetoothConnectionService {

    String ACTION_DATA_RECEIVE = "smartsaw.intent.DATA_RECEIVE";

    String CONST_DATA = "DATA";

    String CONST_TOPIC = "TOPIC";

    void setActivity(AppCompatActivity activity);

    void setContext(Context context);

    void onCreateBluetooth();

    void onResumeBluetooth();

    void onPauseBluetooth();

    void sendMessageToEmbedded(String msg);

}
