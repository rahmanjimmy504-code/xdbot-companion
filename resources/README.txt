The CI workflow compiles android/java/.../HyperbolusBridge.java into
resources/hyperbolus-bridge.dex before geode-sdk/build-geode-mod runs.
The dex is not checked in: it must be compiled from source for each build.
