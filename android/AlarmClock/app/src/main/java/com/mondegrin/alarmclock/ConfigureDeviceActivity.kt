package com.mondegrin.alarmclock


import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts.StartActivityForResult
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Card
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.ui.Modifier
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.mondegrin.alarmclock.ui.theme.AlarmClockTheme


private lateinit var requestPermissionLauncher: ActivityResultLauncher<String>
private lateinit var permissionExplainLauncher: ActivityResultLauncher<Intent>
private val bleDevicesList = mutableStateListOf<BluetoothDevice>()


class ConfigureDeviceActivity : ComponentActivity() {
    @RequiresApi(Build.VERSION_CODES.S)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val bluetoothManager: BluetoothManager = this.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        val bluetoothAdapter: BluetoothAdapter = bluetoothManager.adapter

        requestPermissionLauncher = registerForActivityResult(RequestPermission()) { isGranted: Boolean ->
            if (isGranted) {
                val message: Toast = Toast.makeText(this, "Permissions have been successfully submitted", Toast.LENGTH_SHORT)
                message.show()
            } else {
                val message: Toast = Toast.makeText(this, "To make the app work correctly, please provide the necessary permissions", Toast.LENGTH_LONG)
                message.show()
                finish()
            }
        }
        permissionExplainLauncher = registerForActivityResult(StartActivityForResult()) { result ->
            if (result.resultCode == RESULT_OK) {
                bleDevicesList.addAll(bluetoothAdapter.bondedDevices)
            } else {
                val message: Toast = Toast.makeText(this, "To make the app work correctly, please provide the necessary permissions", Toast.LENGTH_LONG)
                message.show()
                finish()
            }
        }

        when {
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
                bleDevicesList.addAll(bluetoothAdapter.bondedDevices)
            }
            ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.BLUETOOTH_CONNECT) -> {
                val intent: Intent = Intent(this, PermissionsActivity::class.java)
                intent.putExtra("message", "To make the app work correctly, please provide the necessary permissions")
                intent.putExtra("permission", Manifest.permission.BLUETOOTH_CONNECT)
                permissionExplainLauncher.launch(intent)
            }
            else -> {
                requestPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT)
            }
        }

        enableEdgeToEdge()
        setContent {
            AlarmClockTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    MainLayout(modifier = Modifier.padding(innerPadding), this)
                }
            }
        }
    }
}


private fun openDeviceActivity(context: Context) {

}



@RequiresApi(Build.VERSION_CODES.S)
@Composable
private fun MainLayout(modifier: Modifier, activity: Activity) {
    LazyColumn(modifier = modifier) {
        items(bleDevicesList) { device ->
            BleDeviceCard(modifier = modifier, activity = activity, device = device)
        }
    }
}



@RequiresApi(Build.VERSION_CODES.S)
@Composable
private fun BleDeviceCard(modifier: Modifier, activity: Activity, device: BluetoothDevice) {
    Card(onClick = { openDeviceActivity(activity) }, modifier = modifier) {
        Column(modifier = modifier) {
            when {
                ContextCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
                    Text(text = device.name, modifier = modifier)
                }
                ActivityCompat.shouldShowRequestPermissionRationale(activity, Manifest.permission.BLUETOOTH_CONNECT) -> {
                    val intent: Intent = Intent(activity, PermissionsActivity::class.java)
                    intent.putExtra("message", "To make the app work correctly, please provide the necessary permissions")
                    intent.putExtra("permission", Manifest.permission.BLUETOOTH_CONNECT)
                    permissionExplainLauncher.launch(intent)
                }
                else -> {
                    requestPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT)
                }
            }
        }
    }
}
