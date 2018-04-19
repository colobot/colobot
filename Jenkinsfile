#!/usr/bin/env groovy

pipeline {
    agent { label 'colobot-build' }
    options {
        buildDiscarder(logRotator(artifactDaysToKeepStr: '30', artifactNumToKeepStr: '20'))
    }
    stages {
        stage('Check pull request target') {
            when { changeRequest() }
            steps {
                script {
                    if (env.CHANGE_TARGET == 'master') {
                        throw "This pull request targets the wrong branch. Please reopen the pull request targetting the dev branch."
                    }
                }
            }
        }
        stage('Build') {
            parallel {
                stage('Build Windows') {
                    steps {
                        sh 'mkdir -p build/windows'
                        dir('build/windows') {
                            sh '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX=/install \
                                    -DCMAKE_TOOLCHAIN_FILE=/opt/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake \
                                    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDEV_BUILD=1 -DPORTABLE=1 -DTOOLS=1 -DTESTS=0 ../..
                                make
                                rm -rf install
                                DESTDIR=. make install
                            '''
                        }
                    }
                    post {
                        success {
                            sh 'rm -f windows-debug.zip'
                            zip zipFile: 'windows-debug.zip', archive: true, dir: 'build/windows/install'
                        }
                    }
                }
                
                stage('Build Linux') {
                    steps {
                        sh 'mkdir -p build/linux'
                        dir('build/linux') {
                            sh '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX=/install -DCOLOBOT_INSTALL_BIN_DIR=/install -DCOLOBOT_INSTALL_LIB_DIR=/install -DCOLOBOT_INSTALL_DATA_DIR=/install/data -DCOLOBOT_INSTALL_I18N_DIR=/install/lang  -DCMAKE_SKIP_INSTALL_RPATH=ON \
                                    -DBOOST_STATIC=ON -DGLEW_STATIC=ON -DGLEW_LIBRARY=/usr/lib64/libGLEW.a \
                                    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDEV_BUILD=1 -DPORTABLE=1 -DTOOLS=1 -DTESTS=1 -DDESKTOP=0 ../..
                                make
                                rm -rf install
                                DESTDIR=. make install
                                patchelf --set-rpath '.' install/colobot
                            '''
                        }
                    }
                    post {
                        success {
                            sh 'rm -f linux-debug.zip'
                            zip zipFile: 'linux-debug.zip', archive: true, dir: 'build/linux/install'
                        }
                    }
                }
            }
        }

        stage('Generate docs') {
            steps {
                dir('build/linux') {
                    sh 'make doc'
                }
            }
            post {
                success {
                    publishHTML([reportName: 'Doxygen', reportDir: 'build/linux/doc/html', reportFiles: 'index.html', reportTitles: '', allowMissing: false, alwaysLinkToLastBuild: false, keepAll: false])
                }
            }
        }

        stage('Run tests') {
            steps {
                dir('build/linux') {
                    sh './colobot_ut --gtest_output=xml:gtestresults.xml || true'
                }
                step([$class: 'XUnitBuilder', testTimeMargin: '3000', thresholdMode: 1, thresholds: [[$class: 'FailedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '0'], [$class: 'SkippedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '']], tools: [[$class: 'GoogleTestType', deleteOutputFiles: true, failIfNotNew: true, pattern: 'build/linux/gtestresults.xml', skipNoTestFiles: false, stopProcessingIfError: true]]])
            }
            // TODO: Maybe run Windows tests using wine as well?
        }

        stage('Run colobot-lint') {
            steps {
                copyArtifacts filter: 'build/colobot-lint,build/html_report.tar.gz,Tools/count_errors.py', fingerprintArtifacts: true, projectName: 'colobot/colobot-lint/master', selector: lastSuccessful(), target: 'colobot-lint'
                sh 'chmod +x colobot-lint/Tools/count_errors.py' // TODO: ???
                sh 'mkdir -p build/lint'
                dir('build/lint') {
                    sh 'cmake -DCOLOBOT_LINT_BUILD=1 -DTESTS=1 -DTOOLS=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $WORKSPACE'
                    sh '''#!/bin/bash
set -e +x

# Run colobot-lint

COLOBOT_DIR="$WORKSPACE"
COLOBOT_BUILD_DIR="$WORKSPACE/build/lint"

COLOBOT_LINT_BUILD_DIR="$WORKSPACE/colobot-lint/build"

COLOBOT_LINT_REPORT_FILE="$WORKSPACE/build/lint/colobot_lint_report.xml"

CLANG_PREFIX="/usr/lib/llvm-3.6"

cd "$COLOBOT_LINT_BUILD_DIR"
chmod +x ./colobot-lint

# Workaround for Clang not finding system headers
rm -rf bin/
mkdir -p bin
mv ./colobot-lint ./bin/
rm -f ./lib
ln -s ${CLANG_PREFIX}/lib ./lib

echo "Running colobot-lint"
find "$WORKSPACE" \\( -wholename "$COLOBOT_DIR/src/*.cpp" \
                 -or -wholename "$COLOBOT_DIR/test/unit/*.cpp" \
                 -or -wholename "$COLOBOT_BUILD_DIR/fake_header_sources/src/*.cpp" \
                 -or -wholename "$COLOBOT_BUILD_DIR/fake_header_sources/test/unit/*.cpp" \\) \
        -exec ./bin/colobot-lint \
        -verbose \
        -output-format xml \
        -output-file "$COLOBOT_LINT_REPORT_FILE" \
        -p "$COLOBOT_BUILD_DIR" \
        -project-local-include-path "$COLOBOT_DIR/src" -project-local-include-path "$COLOBOT_BUILD_DIR/src" \
        -license-template-file "$COLOBOT_DIR/LICENSE-HEADER.txt" \
        {} +
                    '''
                    sh '''#!/bin/bash
set -e +x

# Generate HTML report

COLOBOT_LINT_BUILD_DIR="$WORKSPACE/colobot-lint/build"
COLBOT_LINT_REPORT_FILE="$WORKSPACE/build/lint/colobot_lint_report.xml"
HTML_REPORT_DIR="$WORKSPACE/build/lint/html_report"

echo "Generating HTML report"
cd "$COLOBOT_LINT_BUILD_DIR"
rm -rf HtmlReport/
tar -zxf html_report.tar.gz
HtmlReport/generate.py --xml-report "$COLBOT_LINT_REPORT_FILE" --output-dir "$HTML_REPORT_DIR"
                    '''
                    script {
                        retcode = sh script: '''#!/bin/bash
set -e +x

# Update stable/unstable build status

ret=0

COLOBOT_LINT_REPORT_FILE="$WORKSPACE/build/lint/colobot_lint_report.xml"
COLOBOT_LINT_DIR="$WORKSPACE/colobot-lint"

OVERALL_STABLE_RULES=(
    "class naming"
    "code block placement"
    "compile error"
#    "compile warning"
#    "enum naming"
#    "function naming"
    "header file not self-contained"
#    "implicit bool cast"
#    "include style"
#    "inconsistent declaration parameter name"
    "license header"
#    "naked delete"
#    "naked new"
#    "old style function"
    "old-style null pointer"
#    "possible forward declaration"
    "undefined function"
#    "uninitialized field"
#    "uninitialized local variable"
#    "unused forward declaration"
#    "variable naming"
    "whitespace"
)

echo "Checking rule stability (overall)"
for ((i = 0; i < ${#OVERALL_STABLE_RULES[@]}; i++)); do
    rule="${OVERALL_STABLE_RULES[$i]}"
    count="$("$COLOBOT_LINT_DIR/Tools/count_errors.py" --rule-filter="$rule" --xml-report-file "$COLOBOT_LINT_REPORT_FILE")"
    if [ "$count" != "0" ]; then
       echo "UNSTABLE RULE: $rule ($count occurences)"
       ret=1
    fi
done

STABLE_RULES_WITHOUT_CBOT=(
    "class naming"
    "code block placement"
    "compile error"
    "compile warning"
#    "enum naming"
#    "function naming"
    "header file not self-contained"
#    "implicit bool cast"
    "include style"
    "inconsistent declaration parameter name"
    "license header"
    "naked delete"
    "naked new"
#    "old style function"
    "old-style null pointer"
#    "possible forward declaration"
    "undefined function"
    "uninitialized field"
#    "uninitialized local variable"
    "unused forward declaration"
#    "variable naming"
    "whitespace"
)

echo "Checking rule stability (without CBOT)"
for ((i = 0; i < ${#STABLE_RULES_WITHOUT_CBOT[@]}; i++)); do
    rule="${STABLE_RULES_WITHOUT_CBOT[$i]}"
    count="$("$COLOBOT_LINT_DIR/Tools/count_errors.py" --rule-filter="$rule" --file-filter="-.*CBot.*" --xml-report-file "$COLOBOT_LINT_REPORT_FILE")"
    if [ "$count" != "0" ]; then
       echo "UNSTABLE RULE: $rule (without CBOT, $count occurences)"
       ret=1
    fi
done

exit $ret
                        ''', returnStatus: true
                        if (retcode != 0) {
                            currentBuild.result = 'UNSTABLE'
                        }
                    }
                }

                // TODO: cppcheck publisher STILL doesn't have pipeline support
                // There is an open pull request though, merged but no release yet... https://github.com/jenkinsci/cppcheck-plugin/pull/36

                publishHTML([reportName: 'Colobot-lint HTML report', reportDir: 'build/lint/html_report', reportFiles: 'index.html', reportTitles: '', allowMissing: false, alwaysLinkToLastBuild: true, keepAll: true])
            }
        }
    }
}
