#!groovy

// TODO generate build dir based on combo of sdkVersion/phoneOrStore/platform
def cmakeAndMSBuild(generator, msBuildName, phoneOrStore, buildDir, buildType, platform, sdkVersion, args) {
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

	def msBuild = tool(name: msBuildName, type: 'hudson.plugins.msbuild.MsBuildInstallation')
	def raw = bat(returnStdout: true, script: "echo %JavaScriptCore_${sdkVersion}_HOME%").trim()
	def jscHome = raw.split('\n')[-1]
	echo "Setting JavaScriptCore_HOME to ${jscHome}"
	withEnv(["JavaScriptCore_HOME=${jscHome}"]) {
		// Run CMake
		dir("build/${buildDir}") {
			bat "cmake -G \"${generator}\" -DCMAKE_SYSTEM_NAME=Windows${phoneOrStore} -DCMAKE_SYSTEM_VERSION=${sdkVersion} ${args} ${env.WORKSPACE}"
		}

		// Then MSBuild
		bat "\"${msBuild}\" /p:Configuration=${buildType} build/${buildDir}/HAL.sln ${msBuildArgs}"
	}
	// scan for warninsg from MSBuild
	warnings consoleParsers: [[parserName: 'MSBuild']]
}

// '-DCMAKE_SYSTEM_NAME=WindowsPhone' or '-DCMAKE_SYSTEM_NAME=WindowsStore'

def test(dirname) {
	// Run CTest
	dir("build/${dirname}") {
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
			[$class: 'CTestType', deleteOutputFiles: true, failIfNotNew: true, pattern: "build/${dirname}/Testing/**/Test.xml", skipNoTestFiles: false, stopProcessingIfError: true]
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
						cmakeAndMSBuild('Visual Studio 12 2013', 'MSBuild 12.0', 'Phone', 'x86', 'Release', 'x86', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
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
						cmakeAndMSBuild('Visual Studio 12 2013', 'MSBuild 12.0', 'Phone', 'ARM', 'Release', 'ARM', '8.1', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						stash includes: 'build/ARM/Release/HAL.*', name: '8.1-ARM'
					} finally {
						deleteDir()
					}
				}
			},
			'Win 10 Phone x86': {
				node('msbuild-14 && vs2015 && jsc && cmake && gtest && boost && windows-sdk-10') {
					try {
						checkout scm
						cmakeAndMSBuild('Visual Studio 14 2015', 'MSBuild 14.0', 'Phone', 'x86', 'Release', 'x86', '10.0', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						stash includes: 'build/x86/Release/HAL.*', name: '10.0-x86'
					} finally {
						deleteDir()
					}
				}
			},
			'Win 10 Phone ARM': {
				node('msbuild-14 && vs2015 && jsc && cmake && gtest && boost && windows-sdk-10') {
					try {
						checkout scm
						cmakeAndMSBuild('Visual Studio 14 2015', 'MSBuild 14.0', 'Phone', 'ARM', 'Release', 'ARM', '10.0', '-DHAL_DISABLE_TESTS=ON -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						stash includes: 'build/ARM/Release/HAL.*', name: '10.0-ARM'
					} finally {
						deleteDir()
					}
				}
			}
			// TODO: Win 10 Store!
		)
	} // stage('Build')

	stage('Test') {
		parallel(
			'Win 8.1 Phone x86': {
				node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
					try {
						checkout scm
						def testDir = 'test-8.1-phone-x86'
						cmakeAndMSBuild('Visual Studio 12 2013', 'MSBuild 12.0', 'Phone', testDir, 'Debug', 'x86', '8.1', '-DHAL_DISABLE_TESTS=OFF -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						test(testDir)
					} finally {
						deleteDir()
					}
				}
			},
			'Win 10 Phone x86': {
				node('msbuild-14 && vs2015 && jsc && cmake && gtest && boost && windows-sdk-10') {
					try {
						checkout scm
						def testDir = 'test-10-phone-x86'
						cmakeAndMSBuild('Visual Studio 14 2015', 'MSBuild 14.0', 'Phone', testDir, 'Debug', 'x86', '10.0', '-DHAL_DISABLE_TESTS=OFF -DHAL_DEFINE_JSCLASSDEFINITIONEMPTY=OFF')
						test(testDir)
					} finally {
						deleteDir()
					}
				}
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
				// TODO Unstash 10!
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
