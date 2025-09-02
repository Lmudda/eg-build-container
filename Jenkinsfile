pipeline {
    agent {
        docker {
            image 'eg-build-container:latest'
            reuseNode true
        }
    }
    stages {
        stage('Test Docker') {
            steps {
                echo "✅ Jenkins is running inside Docker now"
                sh 'uname -a'
                sh 'ls -la /opt/snapshots'
            }
        }
    }
}
