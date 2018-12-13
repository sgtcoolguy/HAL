#!groovy

def cmakeAndMSBuild(buildDir, buildType, platform, sdkVersion, args) {
	def generator = 'Visual Studio 12 2013'
	def msBuildArgs = ''
	if (platform.equals('ARM')) {
		generator += ' ARM'
		msBuildArgs = '/p:Platform=ARM'
	}
	// Clean the build directory
	if (fileExists("build/${buildDir}")) {
		dir("build/${buildDir}") {
			deleteDir()
		}
	}
	bat "mkdir build\\${buildDir}"

	def msBuild12 = tool(name: 'MSBuild 12.0', type: 'hudson.plugins.msbuild.MsBuildInstallation')
	def raw = bat(returnStdout: true, script: "echo %JavaScriptCore_${sdkVersion}_HOME%").trim()
	def jscHome = raw.split('\n')[-1]
	echo "Setting JavaScriptCore_HOME to ${jscHome}"
	withEnv(["JavaScriptCore_HOME=${jscHome}"]) {
		// Run CMake
		dir("build/${buildDir}") {
			bat "cmake -G \"${generator}\" -D CMAKE_BUILD_TYPE=${buildType} ${args} ${env.WORKSPACE}"
		}

		// Then MSBuild
		bat "\"${msBuild12}\" /p:Configuration=${buildType} build/${buildDir}/HAL.sln ${msBuildArgs}"
	}
	// scan for warninsg from MSBuild
	warnings consoleParsers: [[parserName: 'MSBuild']]
}

def test() {
	// Run CTest
	dir('build/testing') {
		bat '(robocopy Debug examples\\Debug HAL.dll) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
		bat '(robocopy Debug test\\Debug HAL.dll) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
		bat 'ctest.exe --no-compress-output -T Test || verify > NUL'
	}
	step([$class: 'XUnitPublisher',
		testTimeMargin: '3000',
		thresholdMode: 1,
		thresholds: [
			[$class: 'FailedThreshold', failureNewThreshold: '0', failureThreshold: '0', unstableNewThreshold: '0', unstableThreshold: '0'],
			[$class: 'SkippedThreshold', failureNewThreshold: '0', failureThreshold: '0', unstableNewThreshold: '0', unstableThreshold: '0']
		],
		tools: [
			[$class: 'CTestType', deleteOutputFiles: true, failIfNotNew: true, pattern: 'build/testing/Testing/**/Test.xml', skipNoTestFiles: false, stopProcessingIfError: true]
		]
	])
}

// Wrap in timestamper
timestamps {
	stage('Build') {
		parallel(
			'Win 8.1 Phone x86': {
				node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
					try {
						checkout scm
						cmakeAndMSBuild('x86', 'Release', 'x86', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						stash includes: 'build/x86/Release/HAL.*', name: '8.1-x86'
						stash includes: 'build/x86/hal_export.h', name: 'export-header'
						stash includes: 'include/**/*.hpp', name: 'headers'
					} finally {
						deleteDir()
					}
				}
			},
			'Win 8.1 Phone ARM': {
				node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
					try {
						checkout scm
						cmakeAndMSBuild('ARM', 'Release', 'ARM', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						stash includes: 'build/ARM/Release/HAL.*', name: '8.1-ARM'
					} finally {
						deleteDir()
					}
				}
			}
			// TODO: Windows 10
			// 'Win 10 Phone x86': {
			// 	node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
			// 		checkout scm
			// 		cmakeAndMSBuild('ARM', 'Release', 'ARM', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
			// 	}
			// },
			// 'Windows 10 Desktop': {
			// 	node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
			// 		checkout scm
			// 		cmakeAndMSBuild('ARM', 'Release', 'ARM', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
			// 		// stash results!
			// }
			// }
		)
	} // stage('Build')

	stage('Test') {
		parallel(
			'Win 8.1 Phone x86': {
				node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
					try {
						checkout scm
						cmakeAndMSBuild('testing', 'Debug', 'x86', '8.1', '-DHAL_DISABLE_TESTS=OFF -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						test()
					} finally {
						deleteDir()
					}
				}
			// TODO: Windows 10
			// 'Windows 10 Phone': {
			// 	node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
			// 		checkout scm
			// 		cmakeAndMSBuild('testing', 'Debug', 'x86', '8.1', '-DHAL_DISABLE_TESTS=OFF -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
			// 		test()
			// 	}
			// },
			// 'Windows 10 Desktop': {
			// 	node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
			// 		checkout scm
			// 		cmakeAndMSBuild('testing', 'Debug', 'x86', '8.1', '-DHAL_DISABLE_TESTS=OFF -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
			// 		test()
			// 	}
			}
		)
	} // stage('Test')

	stage('Results') {
		node('windows') {
			try {
				// Clean the distribution directory
				if (fileExists('dist/HAL')) {
					dir('dist/HAL') {
						deleteDir()
					}
				}
				unstash 'headers'
				unstash 'export-header'
				unstash '8.1-ARM'
				unstash '8.1-x86'
				bat 'mkdir dist\\HAL'
				bat '(robocopy build\\x86\\Release dist\\HAL\\x86 HAL.*) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy build\\ARM\\Release dist\\HAL\\ARM HAL.*) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy include dist\\HAL\\include *.hpp /e) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy build\\x86 dist\\HAL\\include hal_export.h) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				archiveArtifacts artifacts: 'dist/**/*'
			} finally {
				deleteDir()
			}
		} // node('windows')
	} // stage('Results')
}
