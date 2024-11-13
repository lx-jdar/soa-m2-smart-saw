package com.example.smartsaw;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Intent;
import android.os.Bundle;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MovementActivity extends AppCompatActivity {

    //#region Attributes

    public static final int TIME_TEST_LOADING_PROGRESS_BAR = 3000;
    private ButtonWood buttonLeftMovement;
    private ButtonWood buttonRightMovement;
    private ImageButton buttonBack;
    private ImageButton buttonHome;

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
        buttonLeftMovement = findViewById(R.id.btn_left_movement);
        buttonLeftMovement.setButtonText(getString(R.string.left));
        buttonLeftMovement.setButtonImageResource(R.drawable.btn_wood_left_selector);
        buttonRightMovement = findViewById(R.id.btn_right_movement);
        buttonRightMovement.setButtonText(getString(R.string.rigth));
        buttonRightMovement.setButtonImageResource(R.drawable.btn_wood_right_selector);
        buttonBack = findViewById(R.id.btn_movement_back);
        buttonHome = findViewById(R.id.btn_home);
    }

    private void setListeners() {
        buttonLeftMovement.setButtonOnClickListener(v -> {
            showMovementSawPopups();
        });

        buttonRightMovement.setButtonOnClickListener(v -> {
            showMovementSawPopups();
        });

        buttonBack.setOnClickListener(v -> {
            Intent intent = new Intent(MovementActivity.this, PrecisionActivity.class);
            startActivity(intent);
            finish();
        });

        buttonHome.setOnClickListener(v -> {
            Intent intent = new Intent(MovementActivity.this, OptionsActivity.class);
            startActivity(intent);
            finish();
        });
    }

    private void showMovementSawPopups() {
        buttonLeftMovement.setEnabled(false);
        buttonRightMovement.setEnabled(false);
        buttonBack.setEnabled(false);
        buttonHome.setEnabled(false);
        AlertDialog dialogMovingSaw = getPopupMovingSawPopup();
        dialogMovingSaw.show();
        showCompletedMovementPopup(dialogMovingSaw);
    }

    private AlertDialog getPopupMovingSawPopup() {
        Builder builderMovingSaw = new Builder(this);
        LayoutInflater layoutInflater = this.getLayoutInflater();
        View dialogView = layoutInflater.inflate(R.layout.dialog_movement, null);
        builderMovingSaw.setView(dialogView);
        builderMovingSaw.setCancelable(false);
        return builderMovingSaw.create();
    }

    private void showCompletedMovementPopup(AlertDialog dialogMovingSaw) {
        new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
            dialogMovingSaw.dismiss();
            Builder builderAccept = new Builder(this);
            builderAccept.setTitle(getString(R.string.full_displacement));
            builderAccept.setMessage(getString(R.string.the_displacement_has_ended));
            builderAccept.setIcon(android.R.drawable.ic_dialog_alert);
            builderAccept.setPositiveButton(getString(android.R.string.ok), (dialogMoveCompleted, which) -> {
                dialogMoveCompleted.dismiss();
                buttonLeftMovement.setEnabled(true);
                buttonRightMovement.setEnabled(true);
                buttonBack.setEnabled(true);
                buttonHome.setEnabled(true);
            });
            builderAccept.setCancelable(false);
            builderAccept.show();
        }, TIME_TEST_LOADING_PROGRESS_BAR);
    }

    //#endregion

}
