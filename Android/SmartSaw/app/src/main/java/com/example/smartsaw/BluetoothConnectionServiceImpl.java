package com.example.smartsaw;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class BluetoothConnectionServiceImpl implements BluetoothConnectionService {

    //#region Attributes

    private static BluetoothConnectionServiceImpl instance = null;
    private AppCompatActivity activity;
    private Handler bluetoothIn;
    final int handlerState = 0;
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private final StringBuilder recDataString = new StringBuilder();
    private ConnectedThread mConnectedThread;
    private Context mContext;
    public static final int MULTIPLE_PERMISSIONS = 5;
    private static final UUID BT_DEVICE_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final String[] permissions = new String[]{
            android.Manifest.permission.BLUETOOTH,
            android.Manifest.permission.BLUETOOTH_ADMIN,
            android.Manifest.permission.ACCESS_COARSE_LOCATION,
            android.Manifest.permission.ACCESS_FINE_LOCATION
    };

    //#endregion

    //#region Public Methods

    public static BluetoothConnectionServiceImpl getInstance() {
        if (instance == null) {
            instance = new BluetoothConnectionServiceImpl();
        }
        return instance;
    }

    public void onCreateBluetooth() {
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothIn = handlerMsgHiloPrincipal();
    }

    @SuppressLint("MissingPermission")
    public void onResumeBluetooth() {
        BluetoothDevice device = btAdapter.getRemoteDevice(BluetoothConnectionService.HC06_MAC_ADDRESS);
        try {
            btSocket = createBluetoothSocket(device);
        } catch (IOException e) {
            showToast("La creacción del Socket fallo");
            broadcastMessage(ActivityType.MAIN_ACTIVITY.toString(), "La creacción del Socket fallo");
        }
        try {
            btSocket.connect();
        } catch (IOException e) {
            e.printStackTrace();
            try {
                btSocket.close();
            } catch (IOException e2) {
                //insert code to deal with this
            }
            broadcastMessage(ActivityType.MAIN_ACTIVITY.toString(), "La creacción del Socket fallo");
        }
        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();
    }

    @Override
    public void onPauseBluetooth() {
        try {
            btSocket.close();
        } catch (IOException e2) {
        }
    }

    public void setActivity(AppCompatActivity activity) {
        this.activity = activity;
    }

    public void setContext(Context context) {
        this.mContext = context;
    }

    public static boolean checkPermissions(AppCompatActivity activity) {
        int result;
        List<String> listPermissionsNeeded = new ArrayList<>();
        for (String p : permissions) {
            result = ContextCompat.checkSelfPermission(activity, p);
            if (result != PackageManager.PERMISSION_GRANTED) {
                listPermissionsNeeded.add(p);
            }
        }
        if (!listPermissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(
                    activity,
                    listPermissionsNeeded.toArray(new String[0]),
                    MULTIPLE_PERMISSIONS);
            return false;
        }
        return true;
    }

    public void sendMessageToEmbedded(String msg) {
        mConnectedThread.write(msg);
    }

    //#endregion

    //#region Private Methods

    private Handler handlerMsgHiloPrincipal() {
        return new Handler(Looper.getMainLooper()) {
            public void handleMessage(@NonNull android.os.Message msg) {
                recDataString.delete(0, recDataString.length());
                if (msg.what == handlerState) {
                    String readMessage = (String) msg.obj;
                    recDataString.append(readMessage);
                    int endOfLineIndex = recDataString.indexOf("\r\n");
                    if (endOfLineIndex > 0) {
                        recDataString.substring(0, endOfLineIndex);
                        recDataString.delete(0, recDataString.length());
                    }
                    decodeDataForActivity(recDataString.toString());
                }
            }
        };
    }

    private void decodeDataForActivity(String code) {
        if (EmbeddedCode.OK.toString().equals(code)) {
            broadcastMessage(ActivityType.MAIN_ACTIVITY.toString(), "Connected");
        } else if (EmbeddedCode.SON.toString().equals(code)) {
            broadcastMessage(ActivityType.OPTIONS_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.SOFF.toString().equals(code)) {
            broadcastMessage(ActivityType.OPTIONS_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.SUS.toString().equals(code)) {
            broadcastMessage(ActivityType.OPTIONS_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.ME_ON.toString().equals(code)) {
            broadcastMessage(ActivityType.MOVEMENT_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.ME_OFF.toString().equals(code)) {
            broadcastMessage(ActivityType.MOVEMENT_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.PNOK.toString().equals(code)) {
            broadcastMessage(ActivityType.PRECISION_ACTIVITY.toString(), code);
        }
    }

    @SuppressLint("MissingPermission")
    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        return device.createRfcommSocketToServiceRecord(BT_DEVICE_UUID);
    }

    private BluetoothConnectionServiceImpl() {
    }

    private void showToast(String message) {
        Toast.makeText(this.activity.getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    private void broadcastMessage(String activity, String msg) {
        Intent intent = new Intent(ACTION_DATA_RECEIVE);
        intent.putExtra(BluetoothConnectionService.CONST_TOPIC, activity);
        intent.putExtra(BluetoothConnectionService.CONST_DATA, msg);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
    }

    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
            }
            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[256];
            int bytes;
            while (true) {
                try {
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(String input) {
            byte[] msgBuffer = input.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                showToast("La conexion fallo");
            }
        }
    }

    //#endregion

}