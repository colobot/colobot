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
                
                // TODO: Run colobot-lint
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
                    publishHTML target: [$class: 'HtmlPublisherTarget', reportName: 'Doxygen', reportDir: 'build/linux/doc/html', reportFiles: 'index.html']
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
    }
}
