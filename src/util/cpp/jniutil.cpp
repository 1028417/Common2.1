
#include "util.h"

//#include <jni.h>
//#include <QAndroidJniEnvironment>
//#include <QAndroidJniObject>
#include <QtAndroid>
//#include <QtAndroidExtras>

/*int jniutil::buildSdkVerion()
{
    return QtAndroid::androidSdkVersion();
    return QAndroidJniObject::getStaticObjectField(
            "android/os/Build/VERSION", "SDK_INT", "I;");
}

bool jniutil::checkBuildSdkVerion(cqstr qsVer)
{
    return buildSdkVerion() == QAndroidJniObject::getStaticObjectField(
            "android/os/Build/VERSION_CODES", qsVer, "I;");
}

int jniutil::androidVerion()
{
    return QAndroidJniObject::getStaticObjectField(
            "system/Environment", "SYSTEM_VERSION_CODE", "I;");
}

bool jniutil::androidVerion(cqstr qsVer)
{
    return buildSdkVerion() == QAndroidJniObject::getStaticObjectField(
            "android/os/Build/VERSION_CODES", qsVer, "I;");
}*/

//API 23以上需要动态申请权限
#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0)) // Qt5.10以上
bool jniutil::requestAndroidPermission(cqstr qsPermission)
{
    auto ret = QtAndroid::checkPermission(qsPermission);
    if(QtAndroid::PermissionResult::Granted == ret)
    {
        return true;
    }

    QtAndroid::requestPermissionsSync( QStringList() << qsPermission );

    return QtAndroid::PermissionResult::Granted == QtAndroid::checkPermission(qsPermission);
}
#endif

// Z -- jboolean -- bllean
// I -- jint -- int;
// J -- jlong -- long
bool jniutil::checkMobileConnected()
{
    return QtAndroid::androidActivity().callMethod<jboolean>("checkMobileConnected", "()Z");
}

void jniutil::vibrate()
{
    cauto jsName = QAndroidJniObject::getStaticObjectField(
                "android/content/Context",
                "VIBRATOR_SERVICE",
                "Ljava/lang/String;");

    cauto jniService = QtAndroid::androidActivity().callObjectMethod(
                "getSystemService",
                "(Ljava/lang/String;)Ljava/lang/Object;",
                jsName.object<jstring>());

    jniService.callMethod<void>("vibrate", "(J)V", jlong(100));
}

void jniutil::installApk(cqstr qsApkPath)
{
    cauto jsApkPath = QAndroidJniObject::fromString(qsApkPath);
    QtAndroid::androidActivity().callMethod<void>("installApk",
                "(Ljava/lang/String;)V",
                jsApkPath.object<jstring>());
}

//QAndroidJniObject::callStaticMethod<void>("xmusic/XActivity", "installApk",
