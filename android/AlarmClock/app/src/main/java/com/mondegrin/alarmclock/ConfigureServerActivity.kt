package com.mondegrin.alarmclock

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.ImageView
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.result.contract.ActivityResultContracts.GetContent

class ConfigureServerActivity : ComponentActivity() {
    private lateinit var imageView: ImageView
    private lateinit var selectImageButton: Button
    private lateinit var uploadButton: Button
    private var imageUri: Uri? = null

    // Register the contract for picking an image
    private val pickImageLauncher =
        registerForActivityResult(ActivityResultContracts.GetContent()) { uri: Uri? ->
            uri?.let {
                imageUri = it
                imageView.setImageURI(imageUri)  // Display selected image in ImageView
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.xml.activity_server)

        imageView = findViewById(R.id.imageView)
        selectImageButton = findViewById(R.id.selectImageButton)
        uploadButton = findViewById(R.id.uploadButton)

        // Launch the image picker
        selectImageButton.setOnClickListener {
            pickImageLauncher.launch("image/*")
        }

        // Upload the image to the server
        uploadButton.setOnClickListener {
            imageUri?.let { uri -> uploadImage(uri) }
        }
    }

    private fun uploadImage(uri: Uri) {
        // Your code to upload the image to the server
    }
}
