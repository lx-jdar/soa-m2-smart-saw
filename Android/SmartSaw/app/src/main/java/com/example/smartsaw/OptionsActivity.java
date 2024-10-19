package com.example.smartsaw;

import android.annotation.SuppressLint;

import androidx.appcompat.app.AlertDialog;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class OptionsActivity extends AppCompatActivity implements SensorEventListener {

    //#region Attributes

    private SwitchCompat switchOnOff;
    private ImageButton buttonConfiguration;
    private ButtonWood buttonPositioning;
    private ImageButton buttonBack;
    private SensorManager sensorManager;
    private Sensor accelerometer;
    private boolean isOn = false;
    private static final float SHAKE_THRESHOLD = 12.0f; // Ajusta este valor según la sensibilidad

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
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Registra el listener del sensor cuando la actividad está en primer plano
        sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Detiene el listener cuando la actividad está en segundo plano
        sensorManager.unregisterListener(this);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            float x = event.values[0];
            float y = event.values[1];
            float z = event.values[2];

            // Calcula la magnitud de la aceleración
            float acceleration = (float) Math.sqrt(x * x + y * y + z * z);

            // Si la aceleración excede el umbral, cambia el estado
            if (acceleration > SHAKE_THRESHOLD) {
                isOn = !isOn; // Cambia el estado de ON a OFF y viceversa
                switchOnOff.setChecked(isOn); // Actualiza el Switch en pantalla
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // No es necesario manejar este evento en este caso
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

    private void setListeners() {
        switchOnOff.setOnCheckedChangeListener((buttonView, isChecked) -> {
            boolean isEnabled = !isChecked;
            buttonConfiguration.setEnabled(isEnabled);
            buttonPositioning.setEnabled(isEnabled);
            buttonBack.setEnabled(isEnabled);
            // DESCOMENTAR UNA VEZ APLICADA LA LOGICA DEL UMBRAL VERTICAL
            if (isChecked) {
                switchOnOff.postDelayed(this::showAlertPopupVerticalLimit, 1000);
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

    //#endregion

}