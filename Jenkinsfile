pipeline {
  agent {
    docker {
      image 'ghcr.io/lmudda/eg-build-container:linux'
    }

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
  options {
    skipDefaultCheckout()
  }
}