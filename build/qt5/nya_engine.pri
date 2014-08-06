!isEmpty(NYA_ENGINE_DIR)
{
    CONFIG(debug, debug|release)
    {
        NYA_ENGINE_LIB_DIR = $${NYA_ENGINE_DIR}/bin/qt5_$$basename(QMAKESPEC)_debug
    }
    else
    {
        NYA_ENGINE_LIB_DIR = $${NYA_ENGINE_DIR}/bin/qt5_$$basename(QMAKESPEC)_release
    }
}
else
{
    message("Specify path to nya engine directory before including 'nya_engine.pri' by setting NYA_ENGINE_DIR variable")
}
