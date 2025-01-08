package com.mondegrin.alarmclock


import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.mondegrin.alarmclock.ui.theme.AlarmClockTheme


private lateinit var requestPermissionLauncher: ActivityResultLauncher<String>


class PermissionsActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        requestPermissionLauncher = registerForActivityResult(RequestPermission()) { isGranted: Boolean ->
            setResult(if (isGranted) { RESULT_OK } else { RESULT_CANCELED })
            finish()
        }

        val message: String? = intent.getStringExtra("message")
        if (message == null) {
            setResult(RESULT_CANCELED)
            finish()
        }
        val permission: String? = intent.getStringExtra("permission")
        if (permission == null) {
            setResult(RESULT_CANCELED)
            finish()
        }

        enableEdgeToEdge()
        setContent {
            AlarmClockTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    MainLayout(modifier = Modifier.padding(innerPadding), message = message!!, permission = permission!!, activity = this)
                }
            }
        }
    }
}


@Composable
private fun MainLayout(modifier: Modifier, message: String, permission: String, activity: Activity) {
    Column(modifier = modifier) {
        Text(text = message, modifier = modifier)

        Row(modifier = modifier) {
            Button(
                onClick = {
                    activity.setResult(Activity.RESULT_CANCELED)
                    activity.finish()
                },
                modifier = modifier
            ) {
                Text(text = "no thanks", modifier = modifier)
            }

            Button(
                onClick = {
                    when {
                        ContextCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED -> {
                            activity.setResult(Activity.RESULT_OK)
                        }
                        ActivityCompat.shouldShowRequestPermissionRationale(activity, Manifest.permission.BLUETOOTH_CONNECT) -> {
                            requestPermissionLauncher.launch(permission)
                        }
                        else -> {
                            requestPermissionLauncher.launch(permission)
                        }
                    }
                },
                modifier = modifier
            ) {
                Text(text = "ok", modifier = modifier)
            }
        }
    }
}
