package com.example.smartsaw;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.text.TextUtils;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public class PrecisionActivity extends AppCompatActivity implements BTMessageBroadcastReceiver.BTMessageListener {

    //#region Attributes

    private static int precisionValue=0;
    private EditText newValue;
    private ButtonWood buttonUpdate;
    private ImageButton buttonBack;
    private ImageButton buttonNext;
    private TextView currentValue;

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
            precisionValue = Integer.parseInt(newValueString);
            if (isInputValid(newValueString)) {
                //currentValue.setText(String.valueOf(precisionValue));
                //buttonNext.setEnabled(true);
                connectionBtService.sendMessageToEmbedded(newValueString);
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

    private void processEmbeddedAction(String action) {
        if (EmbeddedCode.PNOK.getValue().equals(action)) {
            //this.setMotionEngineAction(true);
            currentValue.setText(String.valueOf(precisionValue));
            buttonNext.setEnabled(true);
            Toast.makeText(getApplicationContext(), "Actualización Existosa!", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(getApplicationContext(), "Acción desconocida: "+action, Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onReceive(Intent intent) {

        // Modificar la variable personalizada
        String activity = intent.getStringExtra(BluetoothConnectionService.CONST_TOPIC);
        if (activity != null && activity.equals(ActivityType.PRECISION_ACTIVITY.toString())) {
            String valor = intent.getStringExtra(BluetoothConnectionService.CONST_DATA);
            processEmbeddedAction(valor);
            Toast.makeText(getApplicationContext(), "se recibió "+valor, Toast.LENGTH_SHORT).show();
        }
    }

    //#endregion

}
