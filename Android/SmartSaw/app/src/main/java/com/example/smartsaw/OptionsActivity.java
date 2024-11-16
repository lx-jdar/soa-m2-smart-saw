package com.example.smartsaw;

import android.annotation.SuppressLint;
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
import androidx.appcompat.app.AlertDialog;
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
                    connectionBtService.sendMessageToEmbedded("S");
                }
            } else if (x < -THRESHOLD) {
                if (isOn) {
                    isOn = false;
                    switchOnOff.setChecked(false);
                    connectionBtService.sendMessageToEmbedded("S");
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

    private void setSawAction(boolean isChecked) {
        boolean isEnabled = !isChecked;
        buttonConfiguration.setEnabled(isEnabled);
        buttonPositioning.setEnabled(isEnabled);
        buttonBack.setEnabled(isEnabled);
        // DESCOMENTAR UNA VEZ APLICADA LA LOGICA DEL UMBRAL VERTICAL
            /*if (isChecked) {
                switchOnOff.postDelayed(this::showAlertPopupVerticalLimit, 1000);
            }*/
        if (isEnabled) {
            connectionBtService.sendMessageToEmbedded(EmbeddedCode.S.toString());
        } else {
            connectionBtService.sendMessageToEmbedded(EmbeddedCode.T.toString());
        }
    }

    private void setListeners() {
        switchOnOff.setOnCheckedChangeListener((buttonView, isChecked) -> {
            this.setSawAction(isChecked);


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
        new AlertDialog.Builder(this)
                .setTitle(getString(R.string.popup_title_vertical_limit))
                .setMessage(getString(R.string.popup_description_vertical_limit))
                .setPositiveButton(android.R.string.ok, (dialog, which) -> {
                    switchOnOff.setChecked(false);
                    buttonConfiguration.setEnabled(true);
                    buttonPositioning.setEnabled(true);
                    buttonBack.setEnabled(true);
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    private void showToast(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onReceive(Intent intent) {

        // Modificar la variable personalizada
        String activity = intent.getStringExtra("TOPIC");
        if (activity != null && activity.equals("OPTIONS_ACTIVITY")) {
            String valor = intent.getStringExtra("DATA");
            Toast.makeText(getApplicationContext(), "se recibió "+valor, Toast.LENGTH_SHORT).show();
            this.processEmbeddedAction(valor);

        }
    }

    private void processEmbeddedAction(String action) {
        if (EmbeddedCode.SOFF.getValue().equals(action)) {
            this.setSawAction(false);
        } else if (EmbeddedCode.SON.getValue().equals(action)) {
            this.setSawAction(true); // Ejemplo para otro caso
        } else {
            Toast.makeText(getApplicationContext(), "Acción desconocida: "+action, Toast.LENGTH_SHORT).show();
        }
    }

    //#endregion

}