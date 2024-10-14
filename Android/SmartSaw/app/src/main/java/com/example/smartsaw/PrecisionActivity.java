package com.example.smartsaw;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class PrecisionActivity extends AppCompatActivity {

    //#region Attributes

    private EditText newValue;
    private ButtonWood buttonUpdate;
    private ImageButton buttonBack;
    private ImageButton buttonNext;
    private TextView currentValue;

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
        setContentView(R.layout.activity_presicion);
        newValue = findViewById(R.id.field_position);
        buttonUpdate = findViewById(R.id.update_precision);
        currentValue = findViewById(R.id.current_value);
        buttonBack = findViewById(R.id.btn_precision_back);
        buttonNext = findViewById(R.id.btn_precision_next);
        buttonUpdate.setButtonText(getString(R.string.update));
    }

    private void setListeners() {
        buttonUpdate.setButtonOnClickListener(v -> {
            String newValueString = newValue.getText().toString();
            if (isInputValid(newValueString)) {
                currentValue.setText(newValueString);
                buttonNext.setEnabled(true);
            } else {
                newValue.setError(getString(R.string.required_field));
            }
        });

        buttonBack.setOnClickListener(v -> {
            Intent intent = new Intent(PrecisionActivity.this, OptionsActivity.class);
            startActivity(intent);
            finish();
        });

        buttonNext.setOnClickListener(v -> {
            String currentValueString = currentValue.getText().toString();
            if (isInputValid(currentValueString)) {
                Intent intent = new Intent(PrecisionActivity.this, MovementActivity.class);
                startActivity(intent);
                finish();
            } else {
                Toast.makeText(PrecisionActivity.this, getString(R.string.current_value_is_required), Toast.LENGTH_SHORT).show();
            }
        });
    }

    private boolean isInputValid(String input) {
        return !TextUtils.isEmpty(input);
    }

    //#endregion

}
