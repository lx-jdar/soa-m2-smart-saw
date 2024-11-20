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

public class PrecisionActivity extends AppCompatActivity implements BTMessageBroadcastReceiver.BTMessageListener
{

  //#region Attributes

  private static int precisionValue = 0;

  private EditText newValue;
  private ButtonWood buttonUpdate;
  private ImageButton buttonBack;
  private ImageButton buttonNext;
  private TextView currentValue;

  private BluetoothConnectionService connectionBtService;

  //#endregion

  //#region Activity Methods

  @Override
  protected void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    initializeView();
    setListeners();
    setConnectionBluetoothService();
    setBroadcastConfiguration();
  }

  //#endregion

  //#region Private Methods

  private void initializeView()
  {
    setContentView(R.layout.activity_presicion);
    newValue = findViewById(R.id.field_position);
    buttonUpdate = findViewById(R.id.update_precision);
    currentValue = findViewById(R.id.current_value);
    buttonBack = findViewById(R.id.btn_precision_back);
    buttonNext = findViewById(R.id.btn_precision_next);
    buttonUpdate.setButtonText(getString(R.string.update));
    currentValue.setText(String.valueOf(precisionValue));
  }

  private void setListeners()
  {
    buttonUpdate.setButtonOnClickListener(v ->
    {
      String newValueString = newValue.getText().toString();
      precisionValue = Integer.parseInt(newValueString);
      if (isInputValid(newValueString))
      {
        connectionBtService.sendMessageToEmbedded(newValueString);
      } else
      {
        newValue.setError(getString(R.string.required_field));
      }
    });

    buttonBack.setOnClickListener(v ->
    {
      Intent intent = new Intent(PrecisionActivity.this, OptionsActivity.class);
      startActivity(intent);
      finish();
    });

    buttonNext.setOnClickListener(v ->
    {
      String currentValueString = currentValue.getText().toString();
      if (isInputValid(currentValueString))
      {
        Intent intent = new Intent(PrecisionActivity.this, MovementActivity.class);
        startActivity(intent);
        finish();
      } else
      {
        showToast(getString(R.string.current_value_is_required));
      }
    });
  }

  private boolean isInputValid(String input)
  {
    return !TextUtils.isEmpty(input);
  }

  private void processEmbeddedAction(String action)
  {
    if (EmbeddedCode.PNOK.toString().equals(action))
    {
      currentValue.setText(String.valueOf(precisionValue));
      buttonNext.setEnabled(true);
      showToast("Actualización Existosa!");
    } else
    {
      showToast("Acción desconocida: " + action);
    }
  }

  @Override
  public void onReceive(Intent intent)
  {
    String activity = intent.getStringExtra(BluetoothConnectionService.CONST_TOPIC);
    if (activity != null && activity.equals(ActivityType.PRECISION_ACTIVITY.toString()))
    {
      String valor = intent.getStringExtra(BluetoothConnectionService.CONST_DATA);
      processEmbeddedAction(valor);
      showToast("Se recibió " + valor);
    }
  }

  private void setConnectionBluetoothService()
  {
    connectionBtService = BluetoothConnectionServiceImpl.getInstance();
    connectionBtService.setActivity(this);
    connectionBtService.setContext(getApplicationContext());
  }

  private void setBroadcastConfiguration()
  {
    BTMessageBroadcastReceiver receiver = new BTMessageBroadcastReceiver(this);
    IntentFilter filter = new IntentFilter(BluetoothConnectionService.ACTION_DATA_RECEIVE);
    LocalBroadcastManager.getInstance(this).registerReceiver(receiver, filter);
  }

  private void showToast(String message)
  {
    Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
  }

  //#endregion

}
