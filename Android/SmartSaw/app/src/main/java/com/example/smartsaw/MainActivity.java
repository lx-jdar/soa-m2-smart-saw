package com.example.smartsaw;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public class MainActivity extends AppCompatActivity implements BTMessageBroadcastReceiver.BTMessageListener {

    //#region Attributes

    private ButtonWood buttonStartSystem;
    private BluetoothConnectionService connectionBtService;
    private boolean isConnected = false;
    private BTMessageBroadcastReceiver receiver;

    //#endregion

    //#region Activity Methods

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initializeView();
        buttonStartSystem.setButtonOnClickListener(buttonListener);
        if (BluetoothConnectionServiceImpl.checkPermissions(this)) {
            connectionBtService = BluetoothConnectionServiceImpl.getInstance();
            connectionBtService.setActivity(this);
            connectionBtService.setContext(getApplicationContext());
            connectionBtService.onCreateBluetooth();
            receiver = new BTMessageBroadcastReceiver(this);
            IntentFilter filter = new IntentFilter(BluetoothConnectionService.ACTION_DATA_RECEIVE);
            LocalBroadcastManager.getInstance(this).registerReceiver(receiver, filter);
        } else {
            finish();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        connectionBtService.onResumeBluetooth();
        connectionBtService.sendMessageToEmbedded("R");
    }

    @Override
    protected void onPause() {
        super.onPause();
        connectionBtService.onPauseBluetooth();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
    }
    //#endregion

    //#region Private Methods

    private void initializeView() {
        setContentView(R.layout.activity_main);
        buttonStartSystem = findViewById(R.id.btn_start_system);
        buttonStartSystem.setButtonText(getString(R.string.start_system));
        EdgeToEdge.enable(this);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
    }

    View.OnClickListener buttonListener = v -> {
        if (isConnected) {
            Intent intent = new Intent(MainActivity.this, OptionsActivity.class);
            startActivity(intent);
            finish();
        } else {
            Toast.makeText(getApplicationContext(), "Bluetooth No conectado!", Toast.LENGTH_SHORT).show();
        }
    };

    @Override
    public void onReceive(Intent intent) {
        String activity = intent.getStringExtra(BluetoothConnectionService.CONST_TOPIC);
        if (activity != null && activity.equals(ActivityType.MAIN_ACTIVITY.toString())) {
            String valor = intent.getStringExtra(BluetoothConnectionService.CONST_DATA);
            isConnected = (valor != null && valor.equals("Connected"));
        }
    }

    //#endregion

}