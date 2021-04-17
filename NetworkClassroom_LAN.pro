TEMPLATE = subdirs

SUBDIRS += \
    src/StudentClient \
    src/TeacherServer \
    third-party/qtsingleapplication

StudentClient.depend = third-party/qtsingleapplication
TeacherServer.depend = third-party/qtsingleapplication
