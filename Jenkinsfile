#!groovy

def cmakeAndMSBuild(buildDir, buildType, platform, sdkVersion, args) {
	def cmake = tool(name: 'InSearchPath', type: 'hudson.plugins.cmake.CmakeTool')
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
			bat "\"${cmake}\" -G \"${generator}\" -D CMAKE_BUILD_TYPE=${buildType} ${args} ${env.WORKSPACE}"
		}

		// Then MSBuild
		bat "\"${msBuild12}\" /p:Configuration=${buildType} build/${buildDir}/HAL.sln ${msBuildArgs}"
	}
}

// Wrap in timestamper
timestamps {
	// FIXME: Don't tie to the WIndows 8.1 machine, really we just need to ensure we use the JSC built against 8.1
	node('msbuild-12 && jsc && cmake && gtest && boost && windows-sdk-8.1') {
		try {
			stage('Checkout') {
				checkout scm
				// TODO: Do a clean checkout?
			}

			stage('Win 8.1 Phone x86 Tests') {
				cmakeAndMSBuild('testing', 'Debug', 'x86', '8.1', '-DHAL_DISABLE_TESTS=OFF')
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

			stage('Win 8.1 Phone x86') {
				cmakeAndMSBuild('x86', 'Release', 'x86', '8.1', '-DHAL_DISABLE_TESTS=ON')
			}

			stage('Win 8.1 Phone ARM') {
				cmakeAndMSBuild('ARM', 'Release', 'ARM', '8.1', '-DHAL_DISABLE_TESTS=ON')
			}

			// TODO Windows 10 Universal?
			stage('Results') {
				// Clean the distribution directory
				if (fileExists('dist/HAL')) {
					dir('dist/HAL') {
						deleteDir()
					}
				}
				bat 'mkdir dist\\HAL'
				bat '(robocopy build\\x86\\Release dist\\HAL\\x86 HAL.*) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy build\\ARM\\Release dist\\HAL\\ARM HAL.*) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy include dist\\HAL\\include *.hpp /e) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				bat '(robocopy build\\x86 dist\\HAL\\include hal_export.h) ^& IF %ERRORLEVEL% LEQ 3 exit /B 0'
				archiveArtifacts artifacts: 'dist/**/*'
			}
		} catch (e) {
			// if any exception occurs, mark the build as failed
			currentBuild.result = 'FAILURE'
			throw e
		} finally {
			step([$class: 'WsCleanup', notFailBuild: true])
		}
	}
}
