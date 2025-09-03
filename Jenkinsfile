pipeline {
    agent {
        label 'eg-build-container'
    }

    options {
        skipDefaultCheckout()
    }

    stages {
        stage('Checkout Source') {
            steps {
                checkout scm
            }
        }

        stage('Build Project') {
            steps {
                sh 'cmake .'
                sh 'make'
            }
        }
    }
}
