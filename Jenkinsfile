pipeline {
    agent {
        docker {
            image 'ghcr.io/lmudda/eg-build-container:windows'
            reuseNode true
        }
    }
    stages {
        stage('Test Docker') {
            steps {
                echo "✅ Jenkins is running inside Docker now"
                bat 'ver'   // Windows equivalent of "uname -a"
                bat 'dir C:\\opt\\snapshots'
            }
        }
    }
}
