pipeline {
    agent {
        docker {
            image 'ghcr.io/lmudda/eg-build-container:linux'
            reuseNode true
        }
    }
    stages {
        stage('Checkout') {
            steps {
                echo 'Cloning the repository...'
                git branch: 'main', url: 'https://github.com/Lmudda/eg-build-container.git'
            }
        }
        stage('Build') {
            steps {
                echo 'Building the project...'
                // This assumes your makefile is in the root of the repository
                sh 'make all'
            }
        }
        stage('Archive Artifacts') {
            steps {
                echo 'Archiving the .hex file...'
                // Adjust the path to your .hex file as needed
                archiveArtifacts artifacts: '**/*.hex', fingerprint: true
            }
        }
    }
}
