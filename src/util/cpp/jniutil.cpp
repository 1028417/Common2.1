
#include "util.h"

//#include <jni.h>
//#include <QAndroidJniEnvironment>
//#include <QAndroidJniObject>
#include <QtAndroid>
//#include <QtAndroidExtras>

/*int jniutil::buildSdkVerion()
{
    return QtAndroid::androidSdkVersion();
    return QAndroidJniObject::getStaticField(
            "android/os/Build/VERSION", "SDK_INT");
}

bool jniutil::checkBuildSdkVerion(const char *pszVersion)
{
    return buildSdkVerion() == QAndroidJniObject::getStaticField(
            "android/os/Build/VERSION_CODES", pszVersion);
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

static QAndroidJniObject _getService(const char *pszName)
{
    cauto jniName = QAndroidJniObject::getStaticObjectField(
                "android/content/Context",
                pszName,
                "Ljava/lang/String;");
    if (!jniName.isValid())
    {
        return QAndroidJniObject();
    }

    return QtAndroid::androidActivity().callObjectMethod(
                "getSystemService",
                "(Ljava/lang/String;)Ljava/lang/Object;",
                jniName.object<jstring>());
}

// Z -- jboolean -- bllean
// I -- jint -- int;
// J -- jlong -- long
bool jniutil::checkMobileConnected()
{
    return QtAndroid::androidActivity().callMethod<jboolean>("checkMobileConnected");
}

void jniutil::vibrate()
{
    cauto jniService = _getService("VIBRATOR_SERVICE");
    if (!jniService.isValid())
    {
        return;
    }
    jniService.callMethod<void>("vibrate", "(J)V", jlong(100));
}

void jniutil::installApk(cqstr qsApkPath)
{
    cauto jsApkPath = QAndroidJniObject::fromString(qsApkPath);
    QtAndroid::androidActivity().callMethod<void>("installApk",
                "(Ljava/lang/String;)V",
                jsApkPath.object<jstring>());
}
