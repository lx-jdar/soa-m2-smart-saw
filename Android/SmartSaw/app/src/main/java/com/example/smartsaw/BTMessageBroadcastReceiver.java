package com.example.smartsaw;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class BTMessageBroadcastReceiver extends BroadcastReceiver {

  private final BTMessageListener listener;

  public BTMessageBroadcastReceiver(BTMessageListener listener) {
    this.listener = listener;
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    if (BluetoothConnectionService.ACTION_DATA_RECEIVE.equals(intent.getAction())) {
      listener.onReceive(intent);
    }
  }

  public interface BTMessageListener {
    void onReceive(Intent intent);
  }
}
