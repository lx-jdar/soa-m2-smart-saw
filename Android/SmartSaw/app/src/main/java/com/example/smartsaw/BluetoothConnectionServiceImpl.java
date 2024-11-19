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

    private static BluetoothConnectionServiceImpl instance = null;

    private AppCompatActivity activity;
    private Handler bluetoothIn;
    final int handlerState = 0;

    //used to identify handler message
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private final StringBuilder recDataString = new StringBuilder();
    private ConnectedThread mConnectedThread;

    private Context mContext;

    // String for MAC address del Hc05
    private static String address = null;
    public static final int MULTIPLE_PERMISSIONS = 5; // code you want.

    // SPP UUID service  - Funciona en la mayoria de los dispositivos
    private static final UUID BT_DEVICE_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static String[] permissions= new String[] {
            android.Manifest.permission.BLUETOOTH,
            android.Manifest.permission.BLUETOOTH_ADMIN,
            //android.Manifest.permission.BLUETOOTH_CONNECT,
            //android.Manifest.permission.BLUETOOTH_SCAN,
            android.Manifest.permission.ACCESS_COARSE_LOCATION,
            android.Manifest.permission.ACCESS_FINE_LOCATION,
            //android.Manifest.permission.WRITE_EXTERNAL_STORAGE,
            //android.Manifest.permission.READ_PHONE_STATE,
            //android.Manifest.permission.READ_EXTERNAL_STORAGE,
    };

    private BluetoothConnectionServiceImpl() { }

    public void setActivity(AppCompatActivity activity) {
        this.activity = activity;
    }

    public void setContext(Context context) {
        this.mContext = context;
    }

    //#endregion

    public static BluetoothConnectionServiceImpl getInstance() {
        if (instance == null) {
            instance = new BluetoothConnectionServiceImpl();
        }
        return instance;
    }

    public void onCreateBluetooth() {
        // Parte Bluetooth
        //obtengo el adaptador del bluethoot
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        //defino el Handler de comunicacion entre el hilo Principal  el secundario.
        //El hilo secundario va a mostrar informacion al layout atraves utilizando indeirectamente a este handler
        bluetoothIn = HandlerMsgHiloPrincipal();
        //defino los handlers para los botones Apagar y encender
        //btnEncender.setOnClickListener(btnEncenderListener);
        //btnApagar.setOnClickListener(btnApagarListener);
        //        if (checkPermissions())        {
        //enableComponent();        }
        System.out.println("ok");
    }

    //Metodo que crea el socket bluethoot
    @SuppressLint("MissingPermission")
    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        return  device.createRfcommSocketToServiceRecord(BT_DEVICE_UUID);
    }

    //Metodo que chequea si estan habilitados los permisos
    public static boolean checkPermissions(AppCompatActivity activity) {
        int result;
        List<String> listPermissionsNeeded = new ArrayList<>();
        //Se chequea si la version de Android es menor a la 6
        for (String p : permissions) {
            result = ContextCompat.checkSelfPermission(activity, p);
            if (result != PackageManager.PERMISSION_GRANTED) {
                listPermissionsNeeded.add(p);
            }
        }
        if (!listPermissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(
                    activity,
                    listPermissionsNeeded.toArray(new String[listPermissionsNeeded.size()]),
                    MULTIPLE_PERMISSIONS );
            return false;
        }
        return true;
    }

    private void decodeDataForActivity(String code) {
        if (EmbeddedCode.SON.getValue().equals(code)) {
            broadcastMessage(ActivityType.OPTIONS_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.SOFF.getValue().equals(code)) {
            broadcastMessage(ActivityType.OPTIONS_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.ME_ON.getValue().equals(code)) {
            broadcastMessage(ActivityType.MOVEMENT_ACTIVITY.toString(), code);
        } else if (EmbeddedCode.ME_OFF.getValue().equals(code)) {
            broadcastMessage(ActivityType.MOVEMENT_ACTIVITY.toString(), code);
        }
    }

    private Handler HandlerMsgHiloPrincipal() {
        return  new Handler(Looper.getMainLooper()) {
            public void handleMessage(@NonNull android.os.Message msg)
            {
                //si se recibio un msj del hilo secundario
                recDataString.delete(0,recDataString.length());
                if (msg.what == handlerState)
                {
                    //voy concatenando el msj
                    String readMessage = (String) msg.obj;
                    recDataString.append(readMessage);
                    int endOfLineIndex = recDataString.indexOf("\r\n");
                    //cuando recibo toda una linea la muestro en el layout
                    if (endOfLineIndex > 0)
                    {
                        String dataInPrint = recDataString.substring(0, endOfLineIndex);
                        //txtPotenciometro.setText(dataInPrint);
                        recDataString.delete(0, recDataString.length());
                    }
                    decodeDataForActivity(recDataString.toString());
                }
            }
        };
    }

    private void showToast(String message) {
        Toast.makeText(this.activity.getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    @SuppressLint("MissingPermission")
    public void onResumeBluetooth() {
        // super.onResume();
        //Obtengo el parametro, aplicando un Bundle, que me indica la Mac Adress del HC05
//        Intent intent=this.activity.getIntent();
//        Bundle extras=intent.getExtras();
        address= "00:22:02:01:0F:38";
        //extras.getString("Direccion_Bluethoot");
        BluetoothDevice device = btAdapter.getRemoteDevice(address);
        //se realiza la conexion del Bluethoot crea y se conectandose a atraves de un socket
        try
        {
            btSocket = createBluetoothSocket(device);
        }
        catch (IOException e)
        {
            showToast("La creacción del Socket fallo");
            broadcastMessage("MAIN_ACTIVITY","La creacción del Socket fallo");
        }

        // Establish the Bluetooth socket connection.
        try
        {
            btSocket.connect();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            try
            {
                btSocket.close();
            }
            catch (IOException e2)
            {
                //insert code to deal with this
            }
            broadcastMessage("MAIN_ACTIVITY","La creacción del Socket fallo");
        }

        //Una establecida la conexion con el Hc05 se crea el hilo secundario, el cual va a recibir
        // los datos de Arduino atraves del bluethoot
        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();
        //I send a character when resuming.beginning transmission to check device is connected
        //If it is not an exception will be thrown in the write method and finish() will be called
        //mConnectedThread.write("S");
        broadcastMessage("MAIN_ACTIVITY", "Connected");
    }

    public void onPauseBluetooth() {
        //    super.onPause();
        try
        {
            //Don't leave Bluetooth sockets open when leaving activity
            btSocket.close();
        } catch (IOException e2) {
            //insert code to deal with this
        }
    }

    private void broadcastMessage(String activity, String msg) {
        //Se envian los valores sensados por el potenciometro, al bradcast reciever de la activity principal
        Intent intent = new Intent(ACTION_DATA_RECEIVE);
        intent.putExtra("TOPIC", activity);
        intent.putExtra("DATA", msg);

        //mContext.sendBroadcast(intent);
        // Enviar el broadcast local
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);

    }

    public void sendMessageToEmbedded(String msg) {
        mConnectedThread.write(msg);
    }

    public  void onStopBluetooth() {
        // super.onStop();
    }

    public void onRestartBluetooth() {
        // super.onRestart();
    }

    public void onDestroyBluetooth() {
        //   super.onDestroy();
    }
    //#endregion

    //#region Private Methods
    private class ConnectedThread extends Thread    {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        //Constructor de la clase del hilo secundario
        public ConnectedThread(BluetoothSocket socket)
        {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;
            try
            {
                //Create I/O streams for connection
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }
            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        //metodo run del hilo, que va a entrar en una espera activa para recibir los msjs del HC05
        public void run()
        {
            byte[] buffer = new byte[256];
            int bytes;
            //el hilo secundario se queda esperando mensajes del HC05
            while (true)
            {
                try
                {
                    //se leen los datos del Bluethoot
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                    //se muestran en el layout de la activity, utilizando el handler del hilo
                    // principal antes mencionado
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(String input) {
            byte[] msgBuffer = input.getBytes();
            //converts entered String into bytes
            try {
                mmOutStream.write(msgBuffer);
                //write bytes over BT connection via outstream
            } catch (IOException e) {
                //if you cannot write, close the application
                showToast("La conexion fallo");
                //finish();
            }
        }
    }
}