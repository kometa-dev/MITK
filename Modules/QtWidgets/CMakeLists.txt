MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  DEPENDS MitkPlanarFigure MitkOverlays MitkConfig MitkUtilities
  PACKAGE_DEPENDS
    PUBLIC ITK|ITKIOImageBase VTK|vtkGUISupportQt Qt5|Widgets+OpenGL+Core Boost
  SUBPROJECTS MITK-CoreUI
)

add_subdirectory(test)
