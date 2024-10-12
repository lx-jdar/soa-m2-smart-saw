package com.example.smartsaw;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.widget.TextView;

import java.util.Random;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;


public class MainActivity extends AppCompatActivity implements SensorEventListener{

    //#region Attributes

    //#endregion

    //#region Activity Methodsç
    private SensorManager mSensorManager;
    private Sensor temperatureSensor;
    private boolean isSimulating = false; // Para saber si estamos simulando

    private TextView temperaturaText;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        setupButton();
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        temperaturaText=findViewById(R.id.temperatura_text);
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_AMBIENT_TEMPERATURE), SensorManager.SENSOR_DELAY_NORMAL);

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy)
    {

    }

    // Metodo que escucha el cambio de los sensores
    @Override
    public void onSensorChanged(SensorEvent event)
    {

        String txt = "";

        // Cada sensor puede lanzar un thread que pase por aqui
        // Para asegurarnos ante los accesos simult�neos sincronizamos esto

        synchronized (this)
        {

            switch(event.sensor.getType())
            {

                case Sensor.TYPE_AMBIENT_TEMPERATURE :
                    float temperature = event.values[0];

                    if (temperature > 0 && temperature<45) {
                        txt =""+ (int)temperature + "ºC";
                        temperaturaText.setText(txt);
                    }
                    break;
            }
        }
    }


    //#endregion

    //#region Private Methods

    private void setupButton() {
        ButtonWood buttonStartSystem = findViewById(R.id.btn_start_system);
        buttonStartSystem.setButtonText(getString(R.string.start_system));
        buttonStartSystem.setButtonOnClickListener(buttonListener);
    }

    View.OnClickListener buttonListener = v -> {
        Intent intent = new Intent(MainActivity.this, OptionsActivity.class);
        startActivity(intent);
        finish();
    };

    //#endregion

}
