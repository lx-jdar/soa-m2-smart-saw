package com.example.smartsaw;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MovementActivity extends AppCompatActivity {

    //#region Attributes

    public static final int ID_RADIO_BUTTON_NOT_SELECTED = -1;
    private RadioGroup radioGroup;
    private ImageButton buttonBack;
    private ButtonWood buttonMovement;

    //#endregion

    //#region Activity Methods

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initializeView();
        setListeners();
    }

    //#endregion

    //#region Private Methods

    private void initializeView() {
        setContentView(R.layout.activity_movement);
        radioGroup = findViewById(R.id.radio_group_movements);
        buttonBack = findViewById(R.id.btn_movement_back);
        buttonMovement = findViewById(R.id.btn_start_movement);
        buttonMovement.setButtonText(getString(R.string.move));
    }

    private void setListeners() {
        buttonMovement.setButtonOnClickListener(v -> {
            int selectedId = radioGroup.getCheckedRadioButtonId();
            if (selectedId == ID_RADIO_BUTTON_NOT_SELECTED) {
                Toast.makeText(MovementActivity.this, "Por favor, seleccione un sentido de desplazamiento", Toast.LENGTH_SHORT).show();
            } else {
                showMovementPopup();
            }
        });

        buttonBack.setOnClickListener(v -> {
            Intent intent = new Intent(MovementActivity.this, PrecisionActivity.class);
            startActivity(intent);
            finish();
        });
    }

    private void showMovementPopup() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Moviendo")
                .setMessage("El desplazamiento está en proceso. Por favor, espere...")
                .setCancelable(false) // Evita que el diálogo se cierre al tocar fuera
                .setPositiveButton("Aceptar", (dialog, which) -> {
                    // Acción al aceptar (si necesitas alguna)
                    dialog.dismiss(); // Cerrar el popup
                });

        // Mostrar el diálogo
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    //#endregion

}
