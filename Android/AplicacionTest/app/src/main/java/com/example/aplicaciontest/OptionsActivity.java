package com.example.aplicaciontest;

import android.content.Intent;
import android.os.Bundle;
import android.widget.ImageButton;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

public class OptionsActivity extends AppCompatActivity {

    private SwitchCompat switchEncendido;
    private ImageButton botonConfiguracion;
    private BotonSmartSaw botonDesplazamiento;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_options);

        switchEncendido = findViewById(R.id.switch_encendido);
        botonConfiguracion = findViewById(R.id.btn_configuration);
        botonDesplazamiento = findViewById(R.id.btn_iniciar_desplazamiento);

        botonDesplazamiento.setButtonText("Iniciar Desplazamiento");
        botonDesplazamiento.setButtonOnClickListener(v -> { });

        switchEncendido.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                botonConfiguracion.setEnabled(false);
                botonDesplazamiento.setEnabled(false);
            } else {
                botonConfiguracion.setEnabled(true);
                botonDesplazamiento.setEnabled(true);
            }
        });

        botonConfiguracion.setOnClickListener(v -> {
            v.playSoundEffect(android.view.SoundEffectConstants.CLICK);
            Intent intent = new Intent(OptionsActivity.this, ConfigurationActivity.class);
            startActivity(intent);
            finish();
        });

    }

}
