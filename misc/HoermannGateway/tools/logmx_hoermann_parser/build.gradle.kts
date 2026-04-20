plugins {
    id("java")
}

group = "org.selfbus"
version = "1.0-SNAPSHOT"
description = "LogMx Java Parser for the Hoermann garage door gateway debug logs"

val logmxDir = (findProperty("logmxDir") as String?) ?: "C:/Program Files/LogMX2"

val logmxJarFile = "$logmxDir/jar/logmx.jar"
val logmxParserDir = "$logmxDir/parsers/classes"

java {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
}

tasks.withType<JavaCompile> {
    options.release.set(8)
}

repositories {
    mavenCentral()
}

dependencies {
    compileOnly(files(logmxJarFile))
//    testImplementation(platform("org.junit:junit-bom:6.0.0"))
//    testImplementation("org.junit.jupiter:junit-jupiter")
//    testRuntimeOnly("org.junit.platform:junit-platform-launcher")
}

tasks.test {
    useJUnitPlatform()
}

// Extract built JAR to LogMX parsers/classes directory after build
tasks.register<Copy>("install") {
    dependsOn(tasks.jar)
    from(zipTree(tasks.jar.get().archiveFile))
    // exclude JAR metadata – LogMX only needs the .class files
    exclude("META-INF/**")
    into(logmxParserDir)
}

// Remove previously installed class files from the LogMX parsers/classes directory
val uninstallPath = "$logmxParserDir/${group.toString().replace('.', '/')}"
tasks.register<Delete>("uninstall") {
    delete(uninstallPath)
}
