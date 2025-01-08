package com.mondegrin.alarmclock


import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.activity.result.contract.ActivityResultContracts.StartActivityForResult
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.BasicAlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TimePicker
import androidx.compose.material3.TimePickerState
import androidx.compose.material3.rememberTimePickerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Modifier
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.mondegrin.alarmclock.ui.theme.AlarmClockTheme
import java.util.Calendar


private lateinit var requestPermissionLauncher: ActivityResultLauncher<String>
private lateinit var permissionExplainLauncher: ActivityResultLauncher<Intent>

var connectionStatus = mutableStateOf("disconected")
lateinit var bleGatt: BluetoothGatt

val openDialog = mutableStateOf(false)


class DeviceActivity : ComponentActivity() {
    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

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
            if (result.resultCode != RESULT_OK) {
                val message: Toast = Toast.makeText(this, "To make the app work correctly, please provide the necessary permissions", Toast.LENGTH_LONG)
                message.show()
                finish()
            }
        }

        val bleDeviceMacAddress: String? = intent.getStringExtra("mac")
        if (bleDeviceMacAddress == null) finish()

        val bluetoothManager: BluetoothManager = this.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        val bluetoothAdapter: BluetoothAdapter = bluetoothManager.adapter

        val bleDevice: BluetoothDevice = bluetoothAdapter.getRemoteDevice(bleDeviceMacAddress)

        val bleGattCallback: BluetoothGattCallback = object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
                super.onConnectionStateChange(gatt, status, newState)
                Log.d("DEBUG_TAG", "connection state changed to $newState")
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    connectionStatus.value = "connected"
                    when {
                        ContextCompat.checkSelfPermission(this@DeviceActivity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
                            gatt?.discoverServices()
                        }
                        ActivityCompat.shouldShowRequestPermissionRationale(this@DeviceActivity, Manifest.permission.BLUETOOTH_CONNECT) -> {
                            val intent: Intent = Intent(this@DeviceActivity, PermissionsActivity::class.java)
                            intent.putExtra("message", "To make the app work correctly, please provide the necessary permissions")
                            intent.putExtra("permission", Manifest.permission.BLUETOOTH_CONNECT)
                            permissionExplainLauncher.launch(intent)
                        }
                        else -> {
                            requestPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT)
                        }
                    }
                }
                if (newState == BluetoothProfile.STATE_CONNECTING) {
                    connectionStatus.value = "disconnecting"
                }
                if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    connectionStatus.value = "disconnected"
                }
                if (newState == BluetoothProfile.STATE_DISCONNECTING) {
                    connectionStatus.value = "disconnecting"
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
                super.onServicesDiscovered(gatt, status)

                if (status == BluetoothGatt.GATT_SUCCESS) {
                    Log.d("DEBUG_TAG", "new services discovered")
                } else {
                    Log.w("DEBUG_TAG", "onServicesDiscovered not success. status: $status")
                }
            }
        }

        when {
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
                bleGatt = bleDevice.connectGatt(this, false, bleGattCallback)
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


@OptIn(ExperimentalMaterial3Api::class)
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
private fun onTimePickedSuccess(activity: Activity, timePickerState: TimePickerState) {
    if (connectionStatus.value != "connected") {
        val message: Toast = Toast.makeText(activity, "The device is not connected", Toast.LENGTH_SHORT)
        message.show()
        return
    }
    val characteristic = bleGatt.services[2].characteristics[0]

    when {
        ContextCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
            val hours: Byte = timePickerState.hour.toByte()
            val minutes: Byte = timePickerState.minute.toByte()
            bleGatt.writeCharacteristic(characteristic, byteArrayOf(hours, minutes), BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
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

    bleGatt.readCharacteristic(characteristic)
    bleGatt.writeCharacteristic(characteristic, "Hello".toByteArray(), BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
}


@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@Composable
private fun MainLayout(modifier: Modifier, activity: Activity) {
    val status by connectionStatus
    Column(modifier = modifier) {
        Text(text = "Status: $status", modifier = modifier)
        Button(onClick = { openDialog.value = true }, modifier = modifier) { Text(text = "Set alarm clock time", modifier = modifier) }
        PickTime(modifier = modifier, activity = activity)
    }
}


@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun PickTime(modifier: Modifier, activity: Activity) {
    val currentTime = Calendar.getInstance()
    val openDialogState by openDialog

    when {
        openDialogState -> {
            val timePickerState: TimePickerState = rememberTimePickerState(
                initialHour = currentTime.get(Calendar.HOUR_OF_DAY),
                initialMinute = currentTime.get(Calendar.MINUTE),
                is24Hour = true,
            )

            BasicAlertDialog(
                onDismissRequest = { openDialog.value = false },
                modifier = modifier
            ) {
                Column(modifier = modifier) {
                    TimePicker(state = timePickerState, modifier = modifier)

                    Row(modifier = modifier) {
                        Button(
                            onClick = {
                                openDialog.value = false
                            },
                            modifier = modifier
                        ) {
                            Text(text = "Cancel", modifier = modifier)
                        }
                        Button(
                            onClick = {
                                onTimePickedSuccess(activity = activity, timePickerState = timePickerState)
                                openDialog.value = false
                            },
                            modifier = modifier,
                        ) {
                            Text(text = "Confirm", modifier = modifier)
                        }
                    }
                }
            }
        }
    }
}
