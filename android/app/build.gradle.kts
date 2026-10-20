plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "de.smleasy.board"
    compileSdk = 34

    // Ergebnis: SMLEasy-Board-<Version>-<debug|release>.apk
    base.archivesName.set("SMLEasy-Board-3.6.0")

    defaultConfig {
        applicationId = "de.smleasy.board"
        targetSdk = 34
        versionCode = 37
        versionName = "3.6.0"
    }
    // legacy: alte Hardware (z. B. Fire HD 10 7. Gen, Fire OS 5 = Android 5.1); standard: Android 7+.
    // Ergebnis: SMLEasy-Board-<Version>-<legacy|standard>-<debug|release>.apk
    flavorDimensions += "api"
    productFlavors {
        create("legacy") {
            dimension = "api"
            minSdk = 22
        }
        create("standard") {
            dimension = "api"
            minSdk = 24
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
    testOptions {
        unitTests.isReturnDefaultValues = true
    }
}

dependencies {
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.json:json:20240303")
}
