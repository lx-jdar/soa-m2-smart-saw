package com.example.smartsaw;

import android.annotation.SuppressLint;

import androidx.appcompat.app.AlertDialog;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

public class OptionsActivity extends AppCompatActivity {

    //#region Attributes

    private SwitchCompat switchOnOff;
    private ImageButton buttonConfiguration;
    private ButtonWood buttonPositioning;

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
        setContentView(R.layout.activity_options);
        switchOnOff = findViewById(R.id.switch_on_off);
        buttonConfiguration = findViewById(R.id.btn_configuration);
        buttonPositioning = findViewById(R.id.btn_position_saw);
        buttonPositioning.setButtonText(getString(R.string.position_saw));
    }

    private void setListeners() {
        switchOnOff.setOnCheckedChangeListener((buttonView, isChecked) -> {
            boolean isEnabled = !isChecked;
            buttonConfiguration.setEnabled(isEnabled);
            buttonPositioning.setEnabled(isEnabled);
            if (isChecked) {
                switchOnOff.postDelayed(this::showAlertPopupVerticalLimit, 3000);
            }
        });

        buttonConfiguration.setOnClickListener(v -> {
            v.playSoundEffect(android.view.SoundEffectConstants.CLICK);
            Intent intent = new Intent(OptionsActivity.this, ConfigurationActivity.class);
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

    private void showAlertPopupVerticalLimit() {
        new AlertDialog.Builder(this)
                .setTitle(getString(R.string.popup_title_vertical_limit))
                .setMessage(getString(R.string.popup_description_vertical_limit))
                .setPositiveButton(android.R.string.ok, (dialog, which) -> {
                    switchOnOff.setChecked(false);
                    buttonConfiguration.setEnabled(true);
                    buttonPositioning.setEnabled(true);
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    //#endregion

}
