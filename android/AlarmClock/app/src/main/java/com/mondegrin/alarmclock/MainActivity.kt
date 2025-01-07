package com.mondegrin.alarmclock


import android.content.Context
import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import com.mondegrin.alarmclock.ui.theme.AlarmClockTheme


class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            AlarmClockTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    MainLayout(modifier = Modifier.padding(innerPadding))
                }
            }
        }
    }
}


@Composable
private fun MainLayout(modifier: Modifier = Modifier) {
    Row(modifier = modifier) {
        val context: Context = LocalContext.current
        Button(onClick = { openDeviceConfigureActivity(context) }, modifier = modifier) { Text(text = "Configure device", modifier = modifier) }
        Button(onClick = { openServerConfigureActivity(context) }, modifier = modifier) { Text(text = "Configure server", modifier = modifier) }
    }
}


private fun openDeviceConfigureActivity(context: Context) {
    val intent: Intent = Intent(context, ConfigureDeviceActivity::class.java)
    context.startActivity(intent)
}


private fun openServerConfigureActivity(context: Context) {
    val intent: Intent = Intent(context, ConfigureDeviceActivity::class.java)
    context.startActivity(intent)
}
