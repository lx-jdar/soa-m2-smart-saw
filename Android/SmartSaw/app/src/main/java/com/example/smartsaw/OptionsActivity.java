package com.example.smartsaw;

import android.annotation.SuppressLint;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageButton;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public class OptionsActivity extends AppCompatActivity implements SensorEventListener, BTMessageBroadcastReceiver.BTMessageListener {
    //#region Attributes

    private SwitchCompat switchOnOff;
    private ImageButton buttonConfiguration;
    private ButtonWood buttonPositioning;
    private ImageButton buttonBack;
    private SensorManager sensorManager;
    private Sensor accelerometer;
    private boolean isOn = false;
    private static final float THRESHOLD = 5.0f;

    private boolean updateSawAction =false;

    private BluetoothConnectionService connectionBtService;

    private BTMessageBroadcastReceiver receiver;

    //#endregion

    //#region Activity Methods

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initializeView();
        setListeners();

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        if (sensorManager != null) {
            accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        }

        connectionBtService = BluetoothConnectionServiceImpl.getInstance();
        connectionBtService.setActivity(this);
        connectionBtService.setContext(getApplicationContext());

        // Registrar el receptor
        receiver = new BTMessageBroadcastReceiver(this);
        IntentFilter filter = new IntentFilter(BluetoothConnectionService.ACTION_DATA_RECEIVE);
        LocalBroadcastManager.getInstance(this).registerReceiver(receiver,filter);
    }

    @SuppressLint("MissingPermission")
    @Override
    protected void onResume() {
        super.onResume();
        if (accelerometer != null) {
            sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Detiene el listener cuando la actividad está en segundo plano
        sensorManager.unregisterListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Desregistrar el receptor local para evitar fugas de memoria
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            float x = event.values[0];
            if (x > THRESHOLD) {
                if (!isOn) {
                    isOn = true;
                    switchOnOff.setChecked(true);
                    connectionBtService.sendMessageToEmbedded(EmbeddedCode.S.toString());
                }
            } else if (x < -THRESHOLD) {
                if (isOn) {
                    isOn = false;
                    switchOnOff.setChecked(false);
                    connectionBtService.sendMessageToEmbedded(EmbeddedCode.T.toString());
                }
            }
        }
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    //#endregion

    //#region Private Methods

    private void initializeView() {
        setContentView(R.layout.activity_options);
        switchOnOff = findViewById(R.id.switch_on_off);
        buttonConfiguration = findViewById(R.id.btn_configuration);
        buttonPositioning = findViewById(R.id.btn_position_saw);
        buttonBack = findViewById(R.id.btn_options_back);
        buttonPositioning.setButtonText(getString(R.string.position_saw));
    }

    private void setButtonStates(boolean isEnabled) {
        buttonConfiguration.setEnabled(isEnabled);
        buttonPositioning.setEnabled(isEnabled);
        buttonBack.setEnabled(isEnabled);
    }

    private void setSawAction(boolean isChecked) {
        boolean isEnabled = !isChecked;
        setButtonStates(isEnabled);
//        if (isEnabled) {
//            connectionBtService.sendMessageToEmbedded(EmbeddedCode.S.toString());
//        } else {
//            connectionBtService.sendMessageToEmbedded(EmbeddedCode.T.toString());
//        }

        new Thread(() -> simulateProgress()).start();
    }

    private void setListeners() {
        switchOnOff.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!updateSawAction) {
                this.setSawAction(isChecked);
            } else {
                updateSawAction =false;
            }
        });

        buttonConfiguration.setOnClickListener(v -> {
            v.playSoundEffect(android.view.SoundEffectConstants.CLICK);
            Intent intent = new Intent(OptionsActivity.this, ConfigurationActivity.class);
            startActivity(intent);
            finish();
        });

        buttonBack.setOnClickListener(v -> {
            Intent intent = new Intent(OptionsActivity.this, MainActivity.class);
            startActivity(intent);
            finish();
        });

        buttonPositioning.setButtonOnClickListener(btnListener);
    }

    View.OnClickListener btnListener = new View.OnClickListener() {
        @SuppressLint("SetTextI18n")
        @Override
        public void onClick(View v) {
            Intent intent = new Intent(OptionsActivity.this, PrecisionActivity.class);
            startActivity(intent);
            finish();
        }
    };

    @SuppressWarnings("unused")
    private void showAlertPopupVerticalLimit() {
        Builder builderAccept = new Builder(this);
        builderAccept.setTitle(getString(R.string.popup_title_vertical_limit));
        builderAccept.setMessage(getString(R.string.popup_description_vertical_limit));
        builderAccept.setIcon(android.R.drawable.ic_dialog_alert);
        builderAccept.setPositiveButton(getString(android.R.string.ok), (dialogMoveCompleted, which) -> {
            updateSawAction =true;
            dialogMoveCompleted.dismiss();
            switchOnOff.setChecked(false);
            setButtonStates(true);
        });
        builderAccept.setCancelable(false);
        builderAccept.show();
    }

    private void showToast(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onReceive(Intent intent) {
        // Modificar la variable personalizada
        String activity = intent.getStringExtra(BluetoothConnectionService.CONST_TOPIC);
        if (activity != null && activity.equals(ActivityType.OPTIONS_ACTIVITY.toString())) {
            String valor = intent.getStringExtra(BluetoothConnectionService.CONST_DATA);
            Toast.makeText(getApplicationContext(), "se recibió "+valor, Toast.LENGTH_SHORT).show();
            this.processEmbeddedAction(valor);

        }
    }

    private void processEmbeddedAction(String action) {
         if (EmbeddedCode.SON.toString().equals(action)) {
             switchOnOff.setChecked(true);
             setButtonStates(true);
        } else if (EmbeddedCode.SOFF.toString().equals(action)) {
             setButtonStates(false);
             updateSawAction =true;
             switchOnOff.setChecked(false);
         } else if (EmbeddedCode.SUS.toString().equals(action)) {
             showAlertPopupVerticalLimit();
         } else {
            Toast.makeText(getApplicationContext(), "Acción desconocida: "+action, Toast.LENGTH_SHORT).show();
        }
    }

    //#endregion

    private void simulateProgress() {
        for (int i = 0; i <= 100; i++) {
            try {
                // Simula el tiempo de ejecución (por ejemplo, un retraso de 50ms por cada incremento)
                Thread.sleep(50);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            final int progress = i;  // Hacemos que el valor de 'i' sea final
            // Actualiza el progreso en la UI
            //runOnUiThread(() -> processEmbeddedAction("SUS"));  // Usamos 'progress' que es final
        }

        // Después de que el progreso llega al 100%, cierra el diálogo
        runOnUiThread(() -> processEmbeddedAction("SOFF"));

    }

}