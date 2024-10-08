package com.example.aplicaciontest;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

public class ConfigurationActivity extends AppCompatActivity {

    private NumberField velocidadDelMotor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_configuration);
        velocidadDelMotor = findViewById(R.id.campo_velocidad_motor);
    }

}
