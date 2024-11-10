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
            String actity = intent.getStringExtra("TOPIC");
            String valor = intent.getStringExtra("DATA");
            // Usar el valor recibido
            Toast.makeText(context, "Valor recibido: " + valor, Toast.LENGTH_SHORT).show();
            // Notificar al listener
            listener.onReceive(valor);
        }
    }

    public interface BTMessageListener {
        void onReceive(String valor);
    }
}
