package com.example.smartsaw;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

public class BTMessageBroadcastReceiver extends BroadcastReceiver {

    private final BTMessageListener listener;

    public BTMessageBroadcastReceiver(BTMessageListener listener) {
        this.listener = listener;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (BluetoothConnectionService.ACTION_DATA_RECEIVE.equals(intent.getAction())) {
            String activity = intent.getStringExtra("TOPIC");
            if (activity.equals("MAIN_ACTIVITY")) {
                String valor = intent.getStringExtra("DATA");
                // Notificar al listener
                listener.onReceive(valor);
            } else if (activity.equals("OPTIONS_ACTIVITY")) {
                String valor = intent.getStringExtra("DATA");
                // Notificar al listener
                listener.onReceive(valor);
            } else if (activity.equals("MOVEMENTS_ACTIVITY")) {
                String valor = intent.getStringExtra("DATA");
                // Notificar al listener
                listener.onReceive(valor);
            }

        }
    }

    public interface BTMessageListener {
        void onReceive(String valor);
    }
}
