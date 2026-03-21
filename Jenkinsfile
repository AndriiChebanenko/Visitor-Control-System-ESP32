pipeline {
    agent none

    triggers {
        pollSCM('H/5 * * * *')
    }

    stages {
        stage('Checkout') {
            agent { label 'linux-agent' }
            steps {
                git branch: 'devops-lpnu',
                    url: 'https://github.com/AndriiChebanenko/Visitor-Control-System-ESP32.git'
            }
        }

        stage('SonarQube Analysis') {
            agent { label 'built-in' }
            steps {
                script {
                    def scannerHome = tool 'sonar-scanner'

                    withSonarQubeEnv('sonar-server') {
                        sh "${scannerHome}/bin/sonar-scanner"
                    }
                }
            }
        }

        stage('Quality Gate') {
            agent { label 'built-in' }
            steps {
                timeout(time: 5, unit: 'MINUTES') {
                    waitForQualityGate abortPipeline: false
                }
            }
        }

        stage('Build') {
            agent {
                docker {
                    image 'espressif/idf:release-v5.4'
                    args '--entrypoint= -e CCACHE_DISABLE=1'
                    label 'linux-agent'
                }
            }

            steps {
                sh '. $IDF_PATH/export.sh && idf.py build'
            }
        }

        stage('Archive') {
            agent { label 'built-in' }
            steps {
                archiveArtifacts artifacts: 'build/*.bin, build/*.elf', fingerprint: true
            }
        }
    }
}
