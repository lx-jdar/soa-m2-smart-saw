package com.example.smartsaw;

import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.Random;

public class MainActivity extends AppCompatActivity implements SensorEventListener {

    //#region Attributes

    ButtonWood buttonStartSystem;
    private SensorManager sensorManager;
    private TextView temperatureText;
    private final boolean isSimulating = true; // Para saber si estamos simulando
    private Handler simulationHandler;
    private Random random;

    //#endregion

    //#region Activity Methods

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initializeView();
        buttonStartSystem.setButtonOnClickListener(buttonListener);
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

        simulationHandler = new Handler(Looper.getMainLooper());
        random = new Random();
    }

    @Override
    protected void onResume() {
        if (isSimulating) {
            startTemperatureSimulation();
        } else {
            initSensors();
        }
        super.onResume();
    }

    @Override
    protected void onPause() {
        stopSensorsOrSimulation();
        super.onPause();
    }

    @Override
    protected void onStop() {
        stopSensorsOrSimulation();
        super.onStop();
    }

    @Override
    protected void onRestart() {
        if (isSimulating) {
            startTemperatureSimulation();
        } else {
            initSensors();
        }
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        stopSensorsOrSimulation();
        super.onDestroy();
    }

    //#endregion

    //#region Public Methods

    @Override
    public void onSensorChanged(SensorEvent event) {
        String txt;
        synchronized (this) {
            if (event.sensor.getType() == Sensor.TYPE_AMBIENT_TEMPERATURE) {
                float temperature = event.values[0];
                if (temperature > 0 && temperature < 45) {
                    txt = (int) temperature + "ºC";
                    temperatureText.setText(txt);
                }
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    //#endregion

    //#region Protected Methods

    protected void initSensors() {
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_AMBIENT_TEMPERATURE), SensorManager.SENSOR_DELAY_NORMAL);
    }

    //#endregion

    //#region Private Methods

    private void initializeView() {
        setContentView(R.layout.activity_main);
        temperatureText = findViewById(R.id.temperature_text);
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
        Intent intent = new Intent(MainActivity.this, OptionsActivity.class);
        startActivity(intent);
        finish();
    };

    private void stopSensorsOrSimulation() {
        if (isSimulating) {
            simulationHandler.removeCallbacksAndMessages(null);
        } else {
            stopSensors();
        }
    }

    private void stopSensors() {
        sensorManager.unregisterListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_AMBIENT_TEMPERATURE));
    }

    private void startTemperatureSimulation() {
        simulationHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                float simulatedTemperature = 15 + random.nextFloat() * (35 - 15);
                String simulatedText = (int) simulatedTemperature + "ºC";
                temperatureText.setText(simulatedText);
                simulationHandler.postDelayed(this, 1000);
            }
        }, 1000);
    }

    //#endregion

}
