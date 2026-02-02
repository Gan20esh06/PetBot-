package com.example.avatarrobot

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.speech.tts.TextToSpeech
import android.util.Log
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.webkit.JavascriptInterface
import android.webkit.PermissionRequest
import android.webkit.WebChromeClient
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import java.util.Locale
import java.util.UUID

class MainActivity : AppCompatActivity(), TextToSpeech.OnInitListener {
    private val TAG = "AvatarRobot"
    
    private lateinit var webView: WebView
    private var textToSpeech: TextToSpeech? = null
    private var ttsInitialized = false
    
    // Bluetooth
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bluetoothGatt: BluetoothGatt? = null
    private var txCharacteristic: BluetoothGattCharacteristic? = null
    private var isConnected = false
    private val UART_SERVICE_UUID = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
    private val UART_TX_CHAR_UUID = UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
    private val handler = Handler(Looper.getMainLooper())

    // Permission launcher
    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val allGranted = permissions.entries.all { it.value }
        if (allGranted) {
            Log.d(TAG, "All permissions granted")
        } else {
            Log.w(TAG, "Some permissions denied")
            Toast.makeText(this, "Some permissions were denied. App may not work fully.", Toast.LENGTH_LONG).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        try {
            Log.d(TAG, "Starting MainActivity onCreate")
            
            // Set content view from layout
            setContentView(R.layout.activity_main)
            
            // Get WebView from layout
            webView = findViewById(R.id.webView)
            
            Log.d(TAG, "WebView found and initialized")
            
            // Enable full screen / immersive mode
            enableImmersiveMode()
            
            // Initialize Text-to-Speech
            textToSpeech = TextToSpeech(this, this)

            // Configure WebView
            webView.settings.apply {
                javaScriptEnabled = true
                domStorageEnabled = true
                allowFileAccess = true
                allowContentAccess = true
                mediaPlaybackRequiresUserGesture = false
                databaseEnabled = true
                cacheMode = WebSettings.LOAD_DEFAULT
                mixedContentMode = WebSettings.MIXED_CONTENT_ALWAYS_ALLOW
            }
            
            // Add JavaScript interface for TTS
            webView.addJavascriptInterface(AndroidTTSInterface(), "AndroidTTS")
            
            // Add JavaScript interface for Bluetooth
            webView.addJavascriptInterface(AndroidBluetoothInterface(), "AndroidBluetooth")

            // WebViewClient to handle page navigation and errors
            webView.webViewClient = object : WebViewClient() {
                override fun onReceivedError(
                    view: WebView?,
                    request: WebResourceRequest?,
                    error: WebResourceError?
                ) {
                    super.onReceivedError(view, request, error)
                    Log.e(TAG, "WebView error: ${error?.description}")
                }
                
                override fun onPageFinished(view: WebView?, url: String?) {
                    super.onPageFinished(view, url)
                    Log.d(TAG, "Page loaded: $url")
                }
            }

            // WebChromeClient for permissions
            webView.webChromeClient = object : WebChromeClient() {
                override fun onPermissionRequest(request: PermissionRequest?) {
                    Log.d(TAG, "Permission request: ${request?.resources?.joinToString()}")
                    request?.grant(request.resources)
                }
            }

            // Handle back button
            onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
                override fun handleOnBackPressed() {
                    if (webView.canGoBack()) {
                        webView.goBack()
                    } else {
                        finish()
                    }
                }
            })

            // Request permissions
            requestAppPermissions()

            // Load the HTML file
            Log.d(TAG, "Loading avatar_ai.html")
            webView.loadUrl("file:///android_asset/avatar_ai.html")
            
            Log.d(TAG, "onCreate completed successfully")
            
        } catch (e: Exception) {
            Log.e(TAG, "Error in onCreate: ${e.message}", e)
            Toast.makeText(this, "Error initializing app: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }

    private fun enableImmersiveMode() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            )
        }
    }

    private fun requestAppPermissions() {
        val permissions = mutableListOf(
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.CAMERA,
            Manifest.permission.INTERNET
        )

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permissions.add(Manifest.permission.BLUETOOTH_CONNECT)
            permissions.add(Manifest.permission.BLUETOOTH_SCAN)
        } else {
            permissions.add(Manifest.permission.BLUETOOTH)
            permissions.add(Manifest.permission.BLUETOOTH_ADMIN)
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            permissions.add(Manifest.permission.ACCESS_FINE_LOCATION)
        }

        val permissionsToRequest = permissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        if (permissionsToRequest.isNotEmpty()) {
            Log.d(TAG, "Requesting permissions: $permissionsToRequest")
            permissionLauncher.launch(permissionsToRequest.toTypedArray())
        }
    }

    // Bluetooth GATT Callback
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    isConnected = true
                    Log.d(TAG, "Bluetooth connected")
                    handler.post {
                        webView.evaluateJavascript("window.onBluetoothConnected && window.onBluetoothConnected()", null)
                    }
                    gatt.discoverServices()
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    isConnected = false
                    Log.d(TAG, "Bluetooth disconnected")
                    handler.post {
                        webView.evaluateJavascript("window.onBluetoothDisconnected && window.onBluetoothDisconnected()", null)
                    }
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                val service = gatt.getService(UART_SERVICE_UUID)
                if (service != null) {
                    txCharacteristic = service.getCharacteristic(UART_TX_CHAR_UUID)
                    Log.d(TAG, "UART service found")
                    handler.post {
                        Toast.makeText(this@MainActivity, "ESP32 Connected!", Toast.LENGTH_SHORT).show()
                    }
                }
            }
        }
    }

    // BLE Scan Callback
    private val leScanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = result.device
            if (device.name?.contains("ESP32", ignoreCase = true) == true) {
                Log.d(TAG, "Found ESP32 device: ${device.name}")
                bluetoothAdapter?.bluetoothLeScanner?.stopScan(this)
                bluetoothGatt = device.connectGatt(this@MainActivity, false, gattCallback)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e(TAG, "BLE scan failed: $errorCode")
            handler.post {
                Toast.makeText(this@MainActivity, "Scan failed", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        textToSpeech?.shutdown()
        bluetoothGatt?.close()
    }

    override fun onInit(status: Int) {
        if (status == TextToSpeech.SUCCESS) {
            val result = textToSpeech?.setLanguage(Locale.US)
            if (result == TextToSpeech.LANG_MISSING_DATA || result == TextToSpeech.LANG_NOT_SUPPORTED) {
                Log.e(TAG, "TTS language not supported")
            } else {
                ttsInitialized = true
                Log.d(TAG, "TTS initialized successfully")
            }
        } else {
            Log.e(TAG, "TTS initialization failed")
        }
    }

    // JavaScript interface for TTS
    inner class AndroidTTSInterface {
        @JavascriptInterface
        fun speak(text: String) {
            Log.d(TAG, "TTS speak: $text")
            textToSpeech?.speak(text, TextToSpeech.QUEUE_FLUSH, null, null)
        }
        
        @JavascriptInterface
        fun stop() {
            textToSpeech?.stop()
        }
        
        @JavascriptInterface
        fun isAvailable(): Boolean {
            return ttsInitialized
        }
    }
    
    // JavaScript interface for Bluetooth
    inner class AndroidBluetoothInterface {
        @SuppressLint("MissingPermission")
        @JavascriptInterface
        fun connect() {
            Log.d(TAG, "AndroidBluetooth.connect called")
            
            val bluetoothManager = getSystemService(BLUETOOTH_SERVICE) as BluetoothManager
            bluetoothAdapter = bluetoothManager.adapter
            
            if (bluetoothAdapter == null) {
                Log.e(TAG, "Bluetooth not supported")
                handler.post {
                    Toast.makeText(this@MainActivity, "Bluetooth not supported", Toast.LENGTH_SHORT).show()
                }
                return
            }
            
            if (!bluetoothAdapter!!.isEnabled) {
                Log.e(TAG, "Bluetooth is disabled")
                handler.post {
                    Toast.makeText(this@MainActivity, "Please enable Bluetooth", Toast.LENGTH_LONG).show()
                }
                return
            }
            
            Log.d(TAG, "Starting BLE scan...")
            handler.post {
                Toast.makeText(this@MainActivity, "Scanning for ESP32...", Toast.LENGTH_SHORT).show()
            }
            
            // Start scanning
            bluetoothAdapter?.bluetoothLeScanner?.startScan(leScanCallback)
            
            // Stop scan after 10 seconds
            handler.postDelayed({
                bluetoothAdapter?.bluetoothLeScanner?.stopScan(leScanCallback)
                if (!isConnected) {
                    Log.d(TAG, "Scan timeout - ESP32 not found")
                    handler.post {
                        Toast.makeText(this@MainActivity, "ESP32 not found", Toast.LENGTH_SHORT).show()
                    }
                }
            }, 10000)
        }
        
        @SuppressLint("MissingPermission")
        @JavascriptInterface
        fun disconnect() {
            Log.d(TAG, "AndroidBluetooth.disconnect called")
            bluetoothGatt?.disconnect()
            bluetoothGatt?.close()
            bluetoothGatt = null
            isConnected = false
        }
        
        @SuppressLint("MissingPermission")
        @JavascriptInterface
        fun send(command: String) {
            Log.d(TAG, "AndroidBluetooth.send: $command")
            
            if (!isConnected || txCharacteristic == null) {
                Log.e(TAG, "Not connected")
                return
            }
            
            try {
                txCharacteristic?.value = command.toByteArray()
                bluetoothGatt?.writeCharacteristic(txCharacteristic)
                Log.d(TAG, "Command sent: $command")
            } catch (e: Exception) {
                Log.e(TAG, "Failed to send command: ${e.message}")
            }
        }
        
        @JavascriptInterface
        fun isConnected(): Boolean {
            return isConnected
        }
    }
}
