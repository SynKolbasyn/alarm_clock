package com.mondegrin.alarmclock


import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import com.mondegrin.alarmclock.ui.theme.AlarmClockTheme


class ConfigureServerActivity : ComponentActivity() {
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

    val startForResult = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) {
            result: ActivityResult ->
        if (result.resultCode == Activity.RESULT_OK) {
            // открываем поток на чтение по полученному URI
            val intent = result.data
            val myFile = getContentResolver().openInputStream(intent?.data!!)
            if (myFile != null) {

                // читаем данные
                val content = myFile.bufferedReader().readText()
                // демонстрируем имя файла и объем прочитанных данных
                Toast
                    .makeText(this, "File %s, Length %d bytes".format(intent?.data!!.path, content.length), Toast.LENGTH_LONG)
                    .show()
            }
        }
    }

    fun onActionClick(view: View) {
        // настраиваем фильтры intent
        val intent = Intent()
            .setType("*/*")
            .setAction(Intent.ACTION_GET_CONTENT)

        // запускаем контракт
        startForResult.launch(intent)
    }
}


@Composable
private fun MainLayout(modifier: Modifier = Modifier) {
    Text(text = "This is server configure activity", modifier = modifier)
}
