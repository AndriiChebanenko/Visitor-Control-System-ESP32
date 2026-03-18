pipeline {
    agent any

    environment {
        PATH = "$PATH:/usr/bin/docker"
    }

    stages {
        stage('Checkout') {
            steps {
                git branch: 'devops-lpnu',
                    url: 'https://github.com/AndriiChebanenko/Visitor-Control-System-ESP32.git'
            }
        }

        stage('Build') {
            agent {
                docker {
                    image 'espressif/idf:release-v5.4'
                    args '--entrypoint='
                    reuseNode true
                }
            }

            steps {
                sh '. $IDF_PATH/export.sh && idf.py build'
            }
        }

        stage('SonarQube Analysis') {
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
            steps {
                timeout(time: 5, unit: 'MINUTES') {
                    waitForQualityGate abortPipeline: true
                }
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts artifacts: 'build/*.bin, build/*.elf', fingerprint: true
            }
        }
    }
}
