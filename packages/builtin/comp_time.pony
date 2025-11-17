primitive CompTimeWorkingDirectory
primitive CompTimeOutputDirectory
primitive CompTimeAbsoluteDirectory
primitive CompTimeTempDirectory

type CompTimeDirectory is (CompTimeWorkingDirectory | CompTimeOutputDirectory | CompTimeAbsoluteDirectory |
  CompTimeTempDirectory)

primitive CompTime
  fun load_file(loc: CompTimeDirectory, file: String): Array[U8] iso^ =>
    compile_intrinsic

  fun save_file(data: Array[U8] box, loc: CompTimeDirectory, file: String) =>
    compile_intrinsic