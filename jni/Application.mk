# Includes DEPRECATED TARGET CPU PLATFORMS (armeabi mips mips64):
# APP_ABI := all
# (...)
APP_ABI := armeabi-v7a x86 x86_64 arm64-v8a # armeabi GONE for Android 5.0(+)

# Explicit setting ref: https://developer.android.com/ndk/guides/stable_apis
APP_PLATFORM := android-22
