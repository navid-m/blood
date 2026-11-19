entry "./src/main.sx"
version "v0.1.0"
spectre_version "v0.0.18"

build release {
    flags "-release"
    output "./blood"
    env {
        GIT_HASH=$((git rev-parse --short HEAD))
    }
}

build dev {
    flags ""
    output "./blood-dev"
    env {
        GIT_HASH=$((git rev-parse --short HEAD))
    }
}
