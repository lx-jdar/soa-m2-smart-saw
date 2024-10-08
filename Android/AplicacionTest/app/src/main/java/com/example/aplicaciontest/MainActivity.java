package com.example.aplicaciontest;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Process;
import android.util.Log;
import android.view.View;
import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {

    private BotonSmartSaw botonIniciarSistema;

    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "Ejecuta: OnCreate");
        Log.i(TAG, String.format("Process ID: %s", Process.myPid()));

        EdgeToEdge.enable(this);

        setContentView(R.layout.activity_main);

        botonIniciarSistema = findViewById(R.id.btn_iniciar_sistema);
        botonIniciarSistema.setButtonText("Iniciar Sistema");
        botonIniciarSistema.setButtonOnClickListener(btnListener);

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

    }

    View.OnClickListener btnListener = new View.OnClickListener() {
        @SuppressLint("SetTextI18n")
        @Override
        public void onClick(View v) {
            Intent intent = new Intent(MainActivity.this, OptionsActivity.class);
            startActivity(intent);
            finish();
        }
    };

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "Ejecuta: OnStart");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "Ejecuta: OnResume");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG, "Ejecuta: OnStop");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "Ejecuta: OnPause");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Ejecuta: OnDestroy");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.i(TAG, "Ejecuta: OnRestart");
    }

}
