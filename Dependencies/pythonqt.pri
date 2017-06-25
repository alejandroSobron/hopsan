# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext = _d
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
pythonqt_home = $${PWD}/pythonqt
pythonqt_lib = $${pythonqt_home}/lib
libname = PythonQt

defineTest(have_local_pythonqt) {
  exists($${pythonqt_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_pythonqt) {
  unix:system(ldconfig -p | grep -q lib$${libname}) {
    return(true)
  }
  return(false)
}

defineTest(have_pythonqt) {
  have_local_pythonqt() {
    return(true)
  }
  have_system_pythonqt() {
    return(true)
  }
  return(false)
}

have_local_pythonqt() {
  INCLUDEPATH *= $${pythonqt_home}/include
  LIBS *= -L$${pythonqt_lib} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${pythonqt_lib}
  message(Found local PythonQt)
} else:have_system_pythonqt() {
  LIBS += -l$${libname}
  message(Found system ldconfig PythonQt)

}