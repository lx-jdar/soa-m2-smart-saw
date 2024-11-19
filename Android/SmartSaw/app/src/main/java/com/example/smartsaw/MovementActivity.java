package com.example.smartsaw;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.widget.ProgressBar;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public class MovementActivity extends AppCompatActivity implements BTMessageBroadcastReceiver.BTMessageListener  {

    //#region Attributes

    private ButtonWood buttonLeftMovement;
    private ButtonWood buttonRightMovement;
    private ImageButton buttonBack;
    private ImageButton buttonHome;
    private AlertDialog progressDialog;
    private ProgressBar progressBar;

    private BluetoothConnectionService connectionBtService;
    private BTMessageBroadcastReceiver receiver;

    //#endregion

    //#region Activity Methods
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initializeView();
        setListeners();

        connectionBtService = BluetoothConnectionServiceImpl.getInstance();
        connectionBtService.setActivity(this);
        connectionBtService.setContext(getApplicationContext());

        // Registrar el receptor
        receiver = new BTMessageBroadcastReceiver(this);
        IntentFilter filter = new IntentFilter(BluetoothConnectionService.ACTION_DATA_RECEIVE);
        LocalBroadcastManager.getInstance(this).registerReceiver(receiver,filter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Desregistrar el receptor local para evitar fugas de memoria
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
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

    private void disableButtons() {
        buttonLeftMovement.setEnabled(false);
        buttonRightMovement.setEnabled(false);
        buttonBack.setEnabled(false);
        buttonHome.setEnabled(false);
    }

    private void setListeners() {
        buttonLeftMovement.setButtonOnClickListener(v -> {
            disableButtons();
            connectionBtService.sendMessageToEmbedded(EmbeddedCode.I.toString());
            showProgressDialog();
        });

        buttonRightMovement.setButtonOnClickListener(v -> {
            disableButtons();
            connectionBtService.sendMessageToEmbedded(EmbeddedCode.D.toString());
            showProgressDialog();
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

    private void showProgressDialog() {
        if (progressDialog == null) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            LayoutInflater inflater = getLayoutInflater();
            View dialogView = inflater.inflate(R.layout.dialog_movement, null);

            progressBar = dialogView.findViewById(R.id.progress_bar);
            progressBar.setMax(100); // Establecer el máximo (100%) para el progreso
            progressBar.setProgress(0); // Comienza en 0%

            builder.setView(dialogView);
            builder.setCancelable(false); // Evitar que se pueda cerrar tocando fuera del dialogo
            progressDialog = builder.create();
        }

        progressDialog.show();
        // Simula el progreso en un hilo separado
        new Thread(() -> simulateProgress()).start();
    }

    private void simulateProgress() {
        for (int i = 0; i <= 100; i++) {
            try {
                // Simula el tiempo de ejecución (por ejemplo, un retraso de 50ms por cada incremento)
                Thread.sleep(50);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            final int progress = i;  // Hacemos que el valor de 'i' sea final
            // Actualiza el progreso en la UI
            runOnUiThread(() -> progressBar.setProgress(progress));  // Usamos 'progress' que es final
        }

        // Después de que el progreso llega al 100%, cierra el diálogo
        runOnUiThread(() -> dismissProgressDialog());
    }

    private void dismissProgressDialog() {
        if (progressDialog != null && progressDialog.isShowing()) {
            progressDialog.dismiss();
            showCompletedMovementPopup();
        }
    }

    private void showCompletedMovementPopup() {
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
    }

    @Override
    public void onReceive(Intent intent) {

        // Modificar la variable personalizada
        String activity = intent.getStringExtra(BluetoothConnectionService.CONST_TOPIC);
        if (activity != null && activity.equals(ActivityType.MOVEMENT_ACTIVITY.toString())) {
            String valor = intent.getStringExtra(BluetoothConnectionService.CONST_DATA);
            processEmbeddedAction(valor);
            Toast.makeText(getApplicationContext(), "Se recibió "+valor, Toast.LENGTH_SHORT).show();
        }
    }

    private void processEmbeddedAction(String action) {
        if (EmbeddedCode.ME_ON.getValue().equals(action)) {
            //this.setMotionEngineAction(true);
            Toast.makeText(getApplicationContext(), "Desplazamiento en Progreso", Toast.LENGTH_SHORT).show();
        } else if (EmbeddedCode.ME_OFF.getValue().equals(action)) {
            dismissProgressDialog();
        } else {
            Toast.makeText(getApplicationContext(), "Acción desconocida: "+action, Toast.LENGTH_SHORT).show();
        }
    }
    //#endregion

}