if (ANDROID)
    set(ANDROIDJNI_SOURCES
        androidjni_annotations_manifest.xml

        src/labs/naver/androidjni/AbstractMethod.java
        src/labs/naver/androidjni/AccessedByNative.java
        src/labs/naver/androidjni/CalledByNative.java
        src/labs/naver/androidjni/NativeConstructor.java
        src/labs/naver/androidjni/NativeDestructor.java
        src/labs/naver/androidjni/NativeExportMacro.java
        src/labs/naver/androidjni/NativeNamespace.java
        src/labs/naver/androidjni/NativeObjectField.java
    )

    add_jar(androidjni.annotations ${ANDROIDJNI_SOURCES} OUTPUT_DIR ${CMAKE_ANDROID_JAR_DIRECTORIES})
endif ()
