plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "de.smleasy.board"
    compileSdk = 34

    defaultConfig {
        applicationId = "de.smleasy.board"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "0.1.0"
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    // Kamera-Scan ueber den Google-Code-Scanner (keine Kameraberechtigung noetig).
    implementation("com.google.android.gms:play-services-code-scanner:16.1.0")
    // QR aus einem ausgewaehlten Foto dekodieren (ohne Play-Dienste).
    implementation("com.google.zxing:core:3.5.3")
}
